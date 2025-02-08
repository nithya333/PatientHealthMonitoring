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


// var csv_button = document.getElementById("csv-export");
// var rep_button = document.getElementById("csv-report");

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
// document.getElementById('ecg_report').addEventListener('click', generatePDF);


function processCSV() {
    const fileInput = document.getElementById('csvFile');
    const file = fileInput.files[0];
    
    if (!file) {
        alert('Please select a CSV file.');
        return;
    }
    
    const reader = new FileReader();
    
    reader.onload = function(e) {
        const text = e.target.result;
        const data = csvToArray(text);
        display_static_ecg(data);
        console.log(data);
    };
    
    reader.readAsText(file);
}

function csvToArray(str, delimiter = ",") {
    // Split the CSV into rows
    const rows = str.trim().split("\n");
    console.log(rows);
    // Map each row into an array of values
    // return rows.map(row => row.split(delimiter));
    return rows;
}

function display_static_ecg(data) {
     // console.log(ecg_csv_list);
     const filtered_ecg_csv_list = data;
     var yValues = filtered_ecg_csv_list.slice(-1000);
     var xValues = Array.from(Array(yValues.length).keys());
 
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
}

document.getElementById('upload').addEventListener('click', processCSV);