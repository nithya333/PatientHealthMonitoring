#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif
#include "rtc_wdt.h"
#include <soc/timer_group_reg.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "MAX30100.h"


#define WIFI_SSID ""
#define WIFI_PASSWORD ""


/* 2. Define the API Key */
#define API_KEY ""

/* 3. Define the RTDB(Firebase realtime database) URL */
#define DATABASE_URL "" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app


// ********************* Temp Sensor ***********************
#define PIN_LM35       35 // ESP32 pin GIOP35 (ADC0) connected to LM35
// *********************************************************


// ******************** Pulse oximeter **********************
// Sampling is tightly related to the dynamic range of the ADC.
// refer to the datasheet for further info
#define SAMPLING_RATE                       MAX30100_SAMPRATE_100HZ

// The LEDs currents must be set to a level that avoids clipping and maximises the
// dynamic range
#define IR_LED_CURRENT                      MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT                     MAX30100_LED_CURR_27_1MA

// The pulse width of the LEDs driving determines the resolution of
// the ADC (which is a Sigma-Delta).
// set HIGHRES_MODE to true only when setting PULSE_WIDTH to MAX30100_SPC_PW_1600US_16BITS
#define PULSE_WIDTH                         MAX30100_SPC_PW_1600US_16BITS
#define HIGHRES_MODE                        true

#define PERIOD_MS_TAKE_READINGS             500
// #define PERIOD_MS_DETECT_FINGER          1000
#define PERIOD_MS_DETECT_FINGER             100
#define PERIOD_MS_PULSE_SENSE               100
#define PULSE_OXIMETER_DEBUG_MODE           (PULSEOXIMETER_DEBUGGINGMODE_NONE)
// #define PULSE_OXIMETER_DEBUG_MODE           (PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES)

#define FINGER_DETECT_AC_IR_THRESHOLD       5000
#define FINGER_DETECT_AC_RED_THRESHOLD      8000

#define TEMP_COUNTER_THRESHOLD              1
#define ECG_COUNTER_THRESHOLD               8
#define PULSE_COUNTER_THRESHOLD             10

#define WIFI_TASK_STACK_SIZE                (10 * 1024) // 50k - 45724
#define MAIN_TASK_STACK_SIZE                (3 * 1024) // 5k - 3220
#define ECG_TASK_STACK_SIZE                 (2 * 1024) // 5k - 4220
#define SPO2_TASK_STACK_SIZE                (2 * 1024) // 5k - ?

#define WIFI_TASK_PRIORITY                  2
#define MAIN_TASK_PRIORITY                  4
#define ECG_TASK_PRIORITY                   5
#define SPO2_TASK_PRIORITY                  3
// *********************************************************


// ****************** Pulse oximeter ***********************
// Instantiate a PulseOximeter sensor class
PulseOximeter pox;

bool finger_detected = false;
uint32_t tsLastReadings = 0;
uint32_t tsLastDetect = 0;
// *********************************************************

// ************************* ECG ***************************
// ECG Sensor is connected to GPIO 34 (Analog ADC1_CH0) 
const int ecg_PIN = 36;

// *********************************************************

// *********************** Firebase ************************
//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

FirebaseJsonArray tempData;
FirebaseJsonArray hrData;
FirebaseJsonArray spo2Data;
FirebaseJsonArray ecgData;

int counter_temp = 0, counter_pulse = 0, counter_ecg = 0;
int prev_counter_temp = 0, prev_counter_pulse = 0, prev_counter_ecg = 0;

// *********************************************************

// ******************* Multitasking ************************
// Parallel execution : different tasks executed on different processors. ESP32 SoC has two processor cores. 
// Concurrent execution : refers to when multiple tasks make progresses by sharing the single CPU time. 

// FreeRTOS is a lightweight Real-Time Operating System (RTOS) designed for low-performance processors like microcontrollers. 
// An RTOS is a type of operating system with deterministic behavior. That means its timing behavior is determined and is not variable.
// [That task will always take 1 second to execute whenever you run it.]

// Mutex stands for “Mutual Exclusion”. A mutex is simply a shared variable. 
// It is used to protect a common resource from simultaneous access by multiple tasks. 
// A process/task can acquire a mutex lock by calling a function called acquire(), the mutex acquires the locked position. 
// If any other tasks try to acquire a mutex lock the requesting tasks will be put in a wait state called busy waiting. 

// More info available in this : 
// https://www.circuitstate.com/wp-content/cache/all/tutorials/how-to-write-parallel-multitasking-applications-for-esp32-using-freertos-arduino/index.html

TaskHandle_t wifi_task_handle;
TaskHandle_t spo2_task_handle;
TaskHandle_t main_task_handle;
TaskHandle_t ecg_task_handle;

SemaphoreHandle_t pulseMutex = NULL;  // Create a mutex object
SemaphoreHandle_t tempMutex = NULL;  // Create a mutex object
SemaphoreHandle_t ecgMutex = NULL;  // Create a mutex object

int counter_p = 0;  // A shared variable
int counter_e = 0;  // A shared variable
bool ecg_string_transfer = false;  // A shared variable
int counter_e_task = 0;  // A shared variable
String ecg_string_producer = "";
String ecg_string_consumer = "";
uint32_t ecg_mv_counter = 0;

void update_fb();
void detect_finger();
void take_readings();
void temp_sense();
void ecg_readings();
void ecg_fb_set();

void wifi_task (void *pvParameters);  // Declaring a task
void spo2_task (void *pvParameters);  // Declaring a task
void ecg_task (void *pvParameters);  // Declaring a task
void main_task (void *pvParameters);  // Declaring a task

void setup()
{
    Serial.begin(115200);
    fb_setup();
    
    Serial.print("Initializing MAX30100..");
    start_MAX30100();

    tempMutex = xSemaphoreCreateMutex();  // create a mutex object
    pulseMutex = xSemaphoreCreateMutex();  // create a mutex object
    ecgMutex = xSemaphoreCreateMutex();

    /*Syntax for assigning task to a core:
    xTaskCreatePinnedToCore(
                      coreTask,   // Function to implement the task
                      "coreTask", // Name of the task 
                      10000,      // Stack size in words 
                      NULL,       // Task input parameter 
                      0,          // Priority of the task 
                      NULL,       // Task handle. 
                      taskCore);  // Core where the task should run 
    */
   
    // Assisgning 4 tasks in which only wifi_task executes on core 0, others on core 1
    // Reason : Firebase updation of data (wifi_task) takes more time which would delay pox.update otherwise.
    // Make sure to call update as fast as possible
    xTaskCreatePinnedToCore(    wifi_task,      "Wi-Fi Task",   WIFI_TASK_STACK_SIZE,      NULL,        WIFI_TASK_PRIORITY,     &wifi_task_handle,      0);
    xTaskCreatePinnedToCore(    spo2_task,      "Pulse",        SPO2_TASK_STACK_SIZE,       NULL,       SPO2_TASK_PRIORITY,      &spo2_task_handle,      1);
    xTaskCreatePinnedToCore(    ecg_task,       "ECG Task",     ECG_TASK_STACK_SIZE,       NULL,        ECG_TASK_PRIORITY,      &ecg_task_handle,       1);

    delay(200);  // needed to start-up other tasks
    xTaskCreatePinnedToCore(    main_task,      "Main Task",    MAIN_TASK_STACK_SIZE,       NULL,       MAIN_TASK_PRIORITY,     &main_task_handle,      1);

}
//================================================================================//

// this task will periodically lock the mutex, calls pox.update(), measures pulse and unlocks the mutex after execution
void spo2_task (void *pvParameters) 
{
    while (1) 
    {
        // Make sure to call update as fast as possible
        pox.update();
        // pulse_sense_efficient();  // To keep detecting finger, and measure pulse only when finger detected 

        // Measures pulse only after time intervals of PERIOD_MS_PULSE_SENSE for better energy optimisation
        if (millis() - tsLastReadings >  PERIOD_MS_PULSE_SENSE)  
        {
            pulse_sense();  // Measures current heart rate and spo2 and sets it to the FirebaseJsonArray object
            tsLastReadings = millis();
        }
        // display_PPG();
        vTaskDelay(1);
    }
}

//================================================================================//

// this task will periodically lock the mutex, increment the counter_p by 1000 and unlock the mutex
void main_task (void *pvParameters) 
{
    while (1) 
    {
        temp_sense();  // Measures current temperature and sets it to the FirebaseJsonArray object

        if ((counter_e % 5) == 0)
        {
            ecg_fb_set();
        }
        counter_e += 1;

        delay(100);
    }
}
//================================================================================//

//================================================================================//

// this task will periodically lock the mutex, increment the counter_p by 1000 and unlock the mutex
void wifi_task (void *pvParameters) 
{
    while (1) 
    {
        update_fb();
        delay(500);
    }
}
//================================================================================//

void ecg_task (void *pvParameters) {
    while (1) 
    {
        ecg_readings();
        delay(19);
        // delay(2);
    }
}
//================================================================================//

void fb_setup() 
  {
    // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // WiFi.begin(WIFI_SSID_2, WIFI_PASSWORD_2);
    WiFi.begin(WIFI_SSID_5, WIFI_PASSWORD_5);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    config.database_url = DATABASE_URL;



    //////////////////////////////////////////////////////////////////////////////////////////////
    //Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
    //otherwise the SSL connection will fail.
    //////////////////////////////////////////////////////////////////////////////////////////////

    Firebase.begin(DATABASE_URL, API_KEY);

    //Comment or pass false value when WiFi reconnection will control by your code or third party library
    // Firebase.reconnectWiFi(true);

    Firebase.setDoubleDigits(5);  
}

void temp_sense()
{
    // The ADC in your ESP32 has a resolution of 12 bits, meaning that it can detect 4096 (0 to 4095) discrete analog levels. 
    // It will convert input voltages ranging from 0 to 3.3V (operating voltage) into integer values ranging from 0 to 4095. 
    // This results in a resolution of 3.3 volts / 4096 units, or 0.0008 volts (0.8 mV) per unit.
    // This function is used to directly get ADC value for a given pin/ADC channel in millivolts.
    float milliVolt = analogReadMilliVolts(PIN_LM35);  
    float tempC = milliVolt/10.0;
    float tempF = tempC * 9.0 / 5.0 + 32;

    if (xSemaphoreTake (tempMutex, 10)) // Take the mutex only if free. Don't wait longer than 10 ticks if busy
    {
        tempData.set("/[" + String(counter_temp) + "]", tempC);  // Appends current temp value to FirebaseJsonArray object
        counter_temp += 1;
        xSemaphoreGive (tempMutex);  // Release the mutex
    }
}

void start_MAX30100() {
  // Initialize the sensor
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin(PULSE_OXIMETER_DEBUG_MODE)) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    
    // Set up the wanted parameters
    // sensor.setMode(MAX30100_MODE_SPO2_HR);
    // sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
    // sensor.setLedsPulseWidth(PULSE_WIDTH);
    // sensor.setSamplingRate(SAMPLING_RATE);
    // sensor.setHighresModeEnabled(HIGHRES_MODE);
}

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("\n Beat!");
}

void pulse_sense_efficient() 
{
    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"

    // If finger is detected, then take_readings of pulse, spo2 in time intervals of PERIOD_MS_TAKE_READINGS ms
    if (finger_detected && (millis() - tsLastReadings >  PERIOD_MS_TAKE_READINGS))
    {
        take_readings();
        tsLastReadings = millis();
        Serial.println("\n\t\t\t\tFinger detected = true, take readings now...........");
    }

    // If finger is not detected, then detect_finger in time intervals of PERIOD_MS_DETECT_FINGER ms
    if (!finger_detected && (millis() - tsLastDetect >  PERIOD_MS_DETECT_FINGER))
    {
        detect_finger();
        tsLastDetect = millis();
        Serial.println("\n\t\t\t\tFinger detected = false, detect finger now...........");
    }
}

void detect_finger() 
{
    if (abs(pox.acIRValue) > FINGER_DETECT_AC_IR_THRESHOLD && abs(pox.acRedValue) > FINGER_DETECT_AC_RED_THRESHOLD) 
    {
        // if ((pox.acRedValue > 0) && (pox.acIRValue > 0) && !finger_detected)
        if ((pox.acRedValue > 0) && (pox.acIRValue > 0))
        {
            finger_detected = true;
            Serial.println("Something is there!");
            // take_readings();
        }
        // else if ((pox.acRedValue < 0) && (pox.acIRValue < 0) && finger_detected)
        // {
        //     finger_detected = false;
        //     Serial.println("Something is not there!");
        // }
    }
  
}

void take_readings() 
{
    Serial.println("\n\t\t\t\tTaking readings...........");
    float heart_rate = pox.getHeartRate();
    uint8_t spo2 = pox.getSpO2();

    // if ((spo2 > 80) && (spo2 <= 100))
    if (heart_rate < 20  && spo2 < 20)
    {
        finger_detected = false;      
    }
    else
    {
        if (xSemaphoreTake (pulseMutex, 0)) // Take the mutex only if free. Don't wait at all if busy
        {
            Serial.print("Heart rate:");
            Serial.print(heart_rate);
            Serial.print("bpm / SpO2:");
            Serial.print(spo2);
            Serial.println("%\n");
            hrData.set("/[" + String(counter_pulse) + "]", heart_rate);
            spo2Data.set("/[" + String(counter_pulse) + "]", spo2);
            counter_pulse += 1 ;
            xSemaphoreGive (pulseMutex);  // Release the mutex
        }
    }
}

void display_PPG()
{
    // This value when plotted gives PPG graph
    Serial.print("$");
    Serial.print(pox.acRedValue - pox.acIRValue);
    Serial.print(";");    
}

void pulse_sense() 
{
    float heart_rate = pox.getHeartRate();
    uint8_t spo2 = pox.getSpO2();

    if (heart_rate > 50 && heart_rate < 100 && spo2 > 85 && spo2 < 100) // Getting only reliable heart rate and spo2
    {
        if (xSemaphoreTake (pulseMutex, 0)) // Take the mutex only if free. Don't wait at all if busy
        {
            hrData.set("/[" + String(counter_pulse) + "]", String(heart_rate, 0));  // Appends current pulse to FirebaseJsonArray object
            spo2Data.set("/[" + String(counter_pulse) + "]", spo2);  // Appends current pulse to FirebaseJsonArray object
            counter_pulse += 1 ;
            xSemaphoreGive (pulseMutex);  // Release the mutex
        }
    }
}

void ecg_readings() {
    uint32_t ecg_mv = analogReadMilliVolts(ecg_PIN);
    // Serial.print("$");
    // Serial.print(ecg_mv);
    // Serial.println(";");

    counter_e_task ++;

    if ((counter_e_task >= 10) && ecg_string_transfer)
    {
        ecg_string_transfer = false;
        counter_e_task = 0;
        ecg_string_consumer = ecg_string_producer;
        ecg_string_producer = "";
    }

  ecg_string_producer += String(ecg_mv) + ",";
}

void ecg_fb_set() 
{
    if (xSemaphoreTake (ecgMutex, 10)) // Take the mutex only if free. Don't wait longer than 10 ticks if busy
    {
        ecgData.set("/[" + String(counter_ecg) + "]", ecg_string_consumer);
        xSemaphoreGive (ecgMutex);  // Release the mutex
        ecg_string_consumer = "";
        ecg_string_transfer = true;
        counter_ecg += 1;
    }
}

void update_fb()
{
    if (Firebase.ready()) 
    {
        // eg: If TEMP_COUNTER_THRESHOLD is 5, then if in case counter_temp = 5 is being missed in an iteration, this makes sure that counter_temp = 6 also becomes T
        // This is done not to miss a large interval for every single miss of a multiple of TEMP_COUNTER_THRESHOLD
        if ((int)(counter_temp / TEMP_COUNTER_THRESHOLD) > prev_counter_temp) 
        {
            if (xSemaphoreTake (tempMutex, portMAX_DELAY))  // try to acquire the mutex 
            {
                Firebase.setArrayAsync(fbdo, "/test/temp", tempData); // Sets the FirebaseJsonArray object to the Firebase data object fbdo. This reflects in the Firebase console on browser
                Serial.print("TEMP updated: ");
                Serial.println(counter_temp);
                prev_counter_temp = (int)(counter_temp / TEMP_COUNTER_THRESHOLD);

                // The temperature here is updated every [TEMP_COUNTER_THRESHOLD = 1] time on the Firebase for real-time temperature
                // But every after 10 values of temp updated, the FirebaseJsonArrayObject tempData is cleared in order to overwrite existing 0 to 9 data cells under temp in Firebase
                // So as to not have all temperature values infinitely appended in firebase
                if (counter_temp >= 10)
                {
                    counter_temp = 0;
                    prev_counter_temp = 0;
                    tempData.clear();
                }
                xSemaphoreGive (tempMutex);  // release the mutex
            }
            else 
            {
                Serial.print ("update_fb: tempMutex not acquired");
                // Serial.println (xTaskGetTickCount());
            }
        }
        
        if ((int)(counter_ecg / ECG_COUNTER_THRESHOLD) > prev_counter_ecg)
        {
            if (xSemaphoreTake (ecgMutex, portMAX_DELAY)) {  // try to acquire the mutex
                Firebase.setArrayAsync(fbdo, "/test/ecg", ecgData);
                Serial.print("ECG updated: ");
                Serial.println(counter_ecg);
                prev_counter_ecg = (int)(counter_ecg / ECG_COUNTER_THRESHOLD);
                // if (counter_ecg >= 21)
                // {
                    counter_ecg = 0;
                    prev_counter_ecg = 0;
                    ecgData.clear();  // Every time the ecg data is updated in Firebase, the FirebaseJsonArray object is cleared to overwrite the same cells next time

                // }
                xSemaphoreGive(ecgMutex);  // release the mutex
            }
            else 
            {
                Serial.print ("update_fb: ecgMutex not acquired");
                // Serial.println (xTaskGetTickCount());
            }
        }

        if ((int)(counter_pulse / PULSE_COUNTER_THRESHOLD) > prev_counter_pulse)
        {
            if (xSemaphoreTake (pulseMutex, portMAX_DELAY)) {  // try to acquire the mutex
                Firebase.setArrayAsync(fbdo, "/test/hr", hrData);
                Firebase.setArrayAsync(fbdo, "/test/spo2", spo2Data);
                Serial.print("PULSE updated: ");
                Serial.println(counter_pulse);
                prev_counter_pulse = (int)(counter_pulse / PULSE_COUNTER_THRESHOLD);
                if (counter_pulse >= 10)  // Same as temp data
                {
                    hrData.clear();
                    spo2Data.clear();
                    counter_pulse = 0;
                    prev_counter_pulse = 0;

                }
                xSemaphoreGive (pulseMutex);  // release the mutex
            }
            else
            {
                Serial.print ("update_fb: pulseMutex not acquired");
                // Serial.println (xTaskGetTickCount());
            }
        }
    }
}


void loop()
{
    // the loop function runs over and over again forever

}
