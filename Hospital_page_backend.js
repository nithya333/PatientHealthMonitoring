// import 'firebase/firestore';
// import { getAuth, onAuthStateChanged } from 'firebase/auth';
// import 'firebase/storage';
// import 'firebase/functions';

/* <script src="https://cdnjs.cloudflare.com/ajax/libs/plotly.js/1.33.1/plotly.min.js" integrity="sha512-V0j9LhrK9IMNdFYZqh+IqU4cjo7wdxyHNyH+L0td4HryBuZ7Oq6QxP2/CWr6TituX31+gv5PnolvERuTbz8UNA==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>    */
// import "https://cdnjs.cloudflare.com/ajax/libs/plotly.js/1.33.1/plotly.min.js";
// import { plot } from "./plotty.min.js";
// import { plot } from "https://cdnjs.cloudflare.com/ajax/libs/plotly.js/1.33.1/plotly.min.js";
// import Plotly from "./plotty.js";
// import * as Plotly from "./plotty.js";
// import Plotly from 'plotly.js-dist'

// Import the functions you need from the SDKs you need
import {getDatabase, ref, child, onValue, get, set, push} from "https://www.gstatic.com/firebasejs/9.8.2/firebase-database.js";
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.8.2/firebase-app.js";
// import "https://www.gstatic.com/charts/loader.js";

// interface ConfigProps {
    //     apiKey: string;
    //     authDomain: string;
    //     databaseURL: string;
    //     projectId: string;
    //     storageBucket: string;
    //     messagingSenderId: string;
    //     appId: string;
    // }
const firebaseConfig = {
    //   copy your firebase config informations
    apiKey: "AIzaSyB0h73QzPXx73nd0CCkU2DDzvpCXQQWR7o",
    authDomain: "sams-108.firebaseapp.com",
    databaseURL: "https://sams-108-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "sams-108",
    storageBucket: "sams-108.appspot.com",
    messagingSenderId: "654794623274",
    appId: "1:654794623274:web:1e60b892398b17c4d5b653",
    measurementId: "G-W1H18SQ73B"
    };


// initialize firebase
const app = initializeApp(firebaseConfig);
if (app.length === 0) {
    console.log("no firebas app")
}else{
    console.log("initialized")
}


const database = getDatabase(app);
const dbRef_ecg = ref(database, 'test/ecg');
const dbRef_spo2 = ref(database, 'test/spo2');
const dbRef_hr = ref(database, 'test/hr');
const dbRef_temp = ref(database, 'test/temp');

var ecg_plot_data_array_counter = 0;
var entire_ecg_list = [];
var ecg_csv_list = [];
var last_temp_value = 30;


var csv_button = document.getElementById("csv-export");
var rep_button = document.getElementById("csv-report");

function getDateTimeString() {
    const date = new Date();
    const year = date.getFullYear();
    const month = String(date.getMonth() + 1).padStart(2, '0'); // Months are zero-indexed
    const day = String(date.getDate()).padStart(2, '0');
    const hours = String(date.getHours()).padStart(2, '0');
    const minutes = String(date.getMinutes()).padStart(2, '0');
    const seconds = String(date.getSeconds()).padStart(2, '0');

    return `${year}${month}${day}_${hours}${minutes}${seconds}`;
}

// Function to download the CSV file
const download = (data) => {
    // Create a Blob with the CSV data and type
    const blob = new Blob([data], { type: 'text/csv' });
    
    // Create a URL for the Blob
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `ECG_Data_${getDateTimeString()}.csv`;
    
    // Trigger the download by clicking the anchor tag
    a.click();
}

csv_button.addEventListener("click", () => {
    // console.log(ecg_csv_list);
    const filtered_ecg_csv_list = ecg_csv_list.filter(item => item !== '');
    const ecg_csv_data = [Object.values(filtered_ecg_csv_list).join('\n')];
    download(ecg_csv_data);
});

rep_button.addEventListener("click", () => {
    // console.log(ecg_csv_list);
    const filtered_ecg_csv_list = ecg_csv_list.filter(item => item !== '');
    // const ecg_csv_data = [Object.values(filtered_ecg_csv_list).join('\n')];
    // download(ecg_csv_data);

    // var xValues = ["Italy", "France", "Spain", "USA", "Argentina"];
    // var yValues = [55, 49, 44, 24, 15];
    var yValues = filtered_ecg_csv_list.slice(-1000);
    var xValues = Array.from(Array(yValues.length).keys());
    var barColors = ["red", "green","blue","orange","brown"];

    new Chart("ecg_report", {
    type: "line",
    width: 100,
    data: {
        labels: xValues,
        datasets: [{
            fill: false,
            lineTension: 0,
            backgroundColor: "rgba(0,0,255,1.0)",
            borderColor: "rgba(0,0,255,0.8)",
            
        data: yValues
        }]
    },
    options: {
        legend: {display: false},
        title: {
        display: true,
        text: "ECG Report"
        }
    }
    });

    // var canvas = document.getElementById('ecgrep');
    // var imgData = canvas.toDataURL("image/jpeg");
    // var pdf = new jsPDF();
  
    // pdf.addImage(imgData, 'JPEG', 0, 0);
    // pdf.save("download.pdf");


    generatePDF();

    //   const coords = xValues.map((x, i) => ({ x, y: yValues[i] }));

    //   new Chart("ecg_report", {
    //     type: "scatter",
    //     data: {
    //       datasets: [{
    //         pointRadius: 2,
    //         pointBackgroundColor: "rgba(0,0,255,1)",
    //         data: coords
    //       }]
    //     },
    //     options: {
    //         legend: {display: false},
    //         title: {
    //         display: true,
    //         text: "ECG Report"
    //         }
    //     }
    //   });
});


// Function to generate PDF using html2pdf.js
function generatePDF() {
// var element = document.getElementById('ecgrep');
// var element = document.getElementById('ecg_report');
var element = document.getElementById('table');
var opt = {
margin:       1,
filename:     'AdminCIE.pdf',
image:        { type: 'jpeg', quality: 0.98 },
html2canvas:  { scale: 2 },
jsPDF:        { unit: 'in', format: 'letter', orientation: 'landscape' }
};
// html2pdf(element);
// New Promise-based usage:
html2pdf().set(opt).from(element).save();
}


// Add an event listener to the "Generate PDF" button
document.getElementById('ecg_report').addEventListener('click', generatePDF);



// When the key values in firebase database under the key "ecg" is changed, it enters this function
onValue(dbRef_ecg, (snapshot) => {
    // All key value pairs under ecg of FB database is given as a list called snapshot.val()
    // "key" gives all keys under the "ecg" and snapshot.val()[key] gives the corresponding value for that "key"
    for(var key in snapshot.val())
    {
        // console.log("snapshot.val" + snapshot.val()[key]);

        // Each value under ecg has a string of numbers separated by ","
        // ecg_list_of_data is an array of all those numbers in the form of string
        var ecg_list_of_data = (snapshot.val()[key]).split(",");

        // ecg_list_of_data.forEach((ecg_data) => console.log(ecg_data));
        ecg_list_of_data.forEach((ecg_data) => parseInt(ecg_data));

        entire_ecg_list.push.apply(entire_ecg_list, ecg_list_of_data);
        ecg_csv_list.push.apply(ecg_csv_list, ecg_list_of_data);
        
        // (document.getElementById("ecg")).innerHTML = entire_ecg_list.length;
        // (document.getElementById("ecg")).innerHTML = entire_ecg_list;
    }    
    }, function(error) {
        // The fetch failed.
        console.error(error);
        });

// When the key values in firebase database under the key "hr" is changed, it enters this function
onValue(dbRef_hr, (snapshot) => {
        // for(var key in snapshot.val()){
        //     console.log("snapshot key" + key);
        //     console.log("snapshot.val" + snapshot.val()[key]);
        // }

        (document.getElementById("hr")).innerHTML = Math.ceil(snapshot.val().at(-1)); // Fetches the last value under "hr"

    }, function(error) {
        // The fetch failed.
        console.error(error);
        });

// When the key values in firebase database under the key "spo2" is changed, it enters this function
onValue(dbRef_spo2, (snapshot) => {

        (document.getElementById("spo2")).innerHTML = snapshot.val().at(-1);

    }, function(error) {
        // The fetch failed.
        console.error(error);
        });

// When the key values in firebase database under the key "temp" is changed, it enters this function
onValue(dbRef_temp, (snapshot) => {
    
        (document.getElementById("temp")).innerHTML = snapshot.val().at(-1);
        drawGauge(snapshot.val().at(-1));

    }, function(error) {
        // The fetch failed.
        console.error(error);
        });


//**************************** ECG Graph with Google Charts API ****************************

google.charts.load('current', {
packages: ['corechart', 'line'],
});
// set callback function when api loaded
google.charts.setOnLoadCallback(drawChart);
function drawChart() {
    // create data object with default value
    let data = google.visualization.arrayToDataTable([
        ['Time', 'ECG'],
        [0, 0],
    ]);
    // create options object with titles, colors, etc.
    let options = {
        title: 'Patient ECG',
        hAxis: {
        textPosition: 'none',
        title: 'Time',
        },
        vAxis: {
        title: 'ECG',
        viewWindow: // Specifies the range of y axis of the ecg graph
        {
            max:1100, 
            min:2400
        }
        },
    };

    // draw chart on load
    let chart = new google.visualization.LineChart(
        document.getElementById('chart')
    );

    chart.draw(data, options);

    // max amount of data rows that should be displayed. 
    // After reaching so many rows, the earliest rows get deleted and new ones get added, causing the graph to be running left
    let maxDatas = 200;    

    let index = 0;

    setInterval(function () {
        let ecg_plot_d = parseInt(entire_ecg_list[ecg_plot_data_array_counter]);
        if (data.getNumberOfRows() > maxDatas) 
            {
                data.removeRows(0, data.getNumberOfRows() - maxDatas);
            }
        data.addRow([index, ecg_plot_d]);
        chart.draw(data, options);
        ecg_plot_data_array_counter++;

        // If the end of entire_ecg_list array is reached, it again displays ecg from beginning of entire_ecg_list array
        if (ecg_plot_data_array_counter >= (entire_ecg_list.length - 1))    
        {
            ecg_plot_data_array_counter = 0;
        }

        index++;
    }, 10); // interval for adding new data every 10ms
}
//******************************************************************************************

//************************ Temperature gauge with Google Charts API ************************
google.charts.load('current', {'packages':['gauge']});
google.charts.setOnLoadCallback(drawGauge);

function drawGauge(plot_val) {
    console.log(plot_val, last_temp_value);
    if (plot_val == undefined)
    {
        last_temp_value = 0;
        plot_val = 0;
    }
    if ((last_temp_value != plot_val))
    {

        var data = google.visualization.arrayToDataTable([
        ['Label', 'Value'],
        ['Temp', last_temp_value]   // Default value from which needle starts moving everytime
        ]);

        var options = {
        // width: 400, height: 120,
        width: 500, height: 150,    // Width and height of gauge
        max: 45,                    // Max value of the scale displayed in gauge
        min: 15,                    // Min value of the scale displayed in gauge
        redFrom: 42, redTo: 45,     // Red colour range on the scale
        yellowFrom:38, yellowTo: 42,    // Yellow colour range on the scale
        greenFrom:30, greenTo: 38,      // Green colour range on the scale
        minorTicks: 5
        };

        var chart = new google.visualization.Gauge(document.getElementById('gauge'));

        chart.draw(data, options);
        data.setValue(0, 1, plot_val);
        chart.draw(data, options);

        last_temp_value = plot_val;

        // setInterval(function() {
        //   data.setValue(0, 1, 40 + Math.round(60 * Math.random()));
        //   chart.draw(data, options);
        // }, 100);
    }
  }
//******************************************************************************************

// If it is required to display many readings each of temp, hr, spo2. i.e, to keep creatng new rows and appending data cells of readings
function AddItemToTable(val_1_ecg, val_2_hr, val_3_spo2, val_4_temp)
{
    tbody.innerHTML = "";
    let trow = document.createElement('tr');
    let td1 = document.createElement('td');
    let td2 = document.createElement('td');
    let td3 = document.createElement('td');
    let td4 = document.createElement('td');

    td1.innerHTML = val_1_ecg;
    td2.innerHTML = val_2_hr;
    td3.innerHTML = val_3_spo2;
    td4.innerHTML = val_4_temp;

    trow.appendChild(td1);
    trow.appendChild(td2);
    trow.appendChild(td3);
    trow.appendChild(td4);

    tbody.appendChild(trow);
}

