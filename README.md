## SAMS: Smart Ambulance Management System with Real-Time Patient Health Monitoring

The Smart Ambulance Management System (SAMS) is an innovative solution designed to optimize medical response times and enhance patient care during emergencies. The system allows hospital authorities to access vital patient health information in real-time and track ambulance locations, ensuring timely medical intervention upon the patient's arrival. This comprehensive approach not only improves the efficiency of ambulance services but also significantly contributes to reducing preventable deaths in emergency situations

## Implementation
The Smart Ambulance Management System (SAMS) integrates both hardware and software components to enhance emergency medical services. Here’s a detailed summary of each:

#### Hardware
Primary Health Sensors: The system includes essential health monitoring devices such as:

* ECG Monitors: These devices track the electrical activity of the heart, providing critical data for cardiac patients.
* Temperature Sensors: They measure the patient's body temperature, helping to identify fever or hypothermia.
* Heart Rate Monitors: These devices continuously monitor the heart rate, which is vital for assessing the patient's cardiovascular status.
* SpO2 Monitors: These sensors measure blood oxygen saturation levels, crucial for respiratory assessment.

#### Software
* Mobile Application: A user-friendly mobile app is developed to facilitate booking of Ambulances: Users can easily request an ambulance through the app.

* Cloud-Based Web Interface: This software component allows hospital authorities to Access Patient Health Data: Vital health information collected from the sensors is relayed to the hospital in real-time.
Monitor Ambulance Location: Hospital staff can track the ambulance's journey, optimizing the response time for medical interventions 

* Data Management System: The software manages the data collected from health sensors and ensures it is securely transmitted to the hospital, enabling healthcare professionals to prepare for the patient's arrival 

## ECG Analysis
This system enhances automated cardiac diagnosis by integrating machine learning, signal processing, and real-time ECG analysis, ensuring accurate and efficient heart disease detection. 

1. Data Collection & Preprocessing
- ECG Signal Acquisition: Collected using the AD8232 sensor.
- Data Upload: ECG signals are uploaded via CSV or image files.
- Noise Removal: Bandpass filtering (5Hz-11Hz) is used to remove noise.

2. Feature Extraction of ECG Signals
- Pan-Tompkins Algorithm: Identifies QRS complexes using derivative signal, squared signal, and moving window integration.
- Time Domain Features: Includes mean, standard deviation, range, interquartile range, skewness, kurtosis, etc.
- Frequency Domain Features:
MFCC Analysis: Extracts spectral power information.

Fast Fourier Transform (FFT): Converts time-domain signal into frequency components.

Wavelet Transform: Captures ECG signal characteristics at multiple scales.

Power Spectral Density (PSD): Analyzes signal power distribution.

- Morphological Features: Amplitudes and durations of P waves, QRS complexes, and T waves are extracted for cardiac analysis.

3. Machine Learning – Deep Learning Model for Classification
- Artificial Neural Network (ANN): Learns ECG signal patterns through backpropagation.
- XGBoost: Utilizes gradient boosting to enhance classification accuracy.
- Multi-Layer Perceptron (MLP): Classifies ECG signals by learning complex relationships.

4. Disease Classification
The trained model predicts five prevalent heart diseases: 
Arrhythmia
Atrial Fibrillation
Ischemia
Wolff-Parkinson-White Syndrome
Cardiac Arrest


## Key Features of the System
* Real-Time Patient Monitoring and Health Analysis: SAMS integrates primary health sensors, such as ECG, temperature, heart rate, and SpO2 monitors, which provide continuous health data. The ML/DL model processes signals and provides instant results.

* Web-Based Platform: Allows users to upload and analyze ECG signals. This information is relayed to hospital authorities in real-time, allowing for immediate medical assessment and preparation before the patient arrives 

* User-Friendly Mobile Application: The system features a mobile application that simplifies the process of booking an ambulance. 

* Cloud-Based Data Management: The use of a cloud-based web interface allows for efficient data management and access. Hospital staff can monitor the ambulance's journey and patient health data seamlessly, ensuring that they are well-prepared for the patient's needs upon arrival 

* Optimized Response Time: By enabling instant access to vital health information and real-time tracking of ambulances, SAMS significantly reduces the time taken for medical assistance to reach patients. This optimization is crucial in emergency situations where every second counts

## IEEE Paper for Reference
Here is the link to our research paper on this for detailed information:

1. [Smart Ambulance Management System with Real-Time Patient Monitoring : M. Nithyashree, K. P. Nadgir, P. N and R. S. Rajendran] (https://ieeexplore.ieee.org/document/10276354)

2. [Real-time Longitudinal ECG Analysis for
Prediction of Cardiac Events : Krupa P Nadgir, M Nithyashree, Narendra Kumar S] (https://www.ijrar.org/papers/IJRAR24C2389.pdf)
