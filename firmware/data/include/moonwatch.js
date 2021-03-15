let printers = 0;


// load settings
function load() {

}


// store wifi 
function storewifi() {
    let ssid = document.getElementById("wifi").value;
    let pw = document.getElementById("password").value;

    let options = {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({
            wifi: ssid,
            password: pw
        })
    }

    fetch("/rest/endpoint", options)
    .then(response => response.json())
    .then(data => console.log(data));

}

// add a new printer printer inputs to config site
function addPrinter() {

    let newPrinter = 0;
    if (printers != 0) {       
        newPrinter = printers++;
    } else {
        printers ++;
    }

    // create table
    let pTable = document.createElement('table');
    pTable.id = newPrinter;
    pTable.className = "tblPrinter";

    // name row
    let nameRow = document.createElement("tr");
    pTable.appendChild(nameRow);

    let labelColName = document.createElement("td");
    labelColName.className = "label";
    labelColName.appendChild(document.createTextNode("Printer Name:"))
    nameRow.appendChild(labelColName);

    let settingsColName = document.createElement("td");
    settingsColName.className = "settings";    
    nameRow.appendChild(settingsColName);

    let inputName = document.createElement("input");
    inputName.id = "printername_" + newPrinter;
    inputName.setAttribute("type", "text");    
    settingsColName.appendChild(inputName);

    // host row
    let hostRow = document.createElement("tr");
    pTable.appendChild(hostRow);

    let labelColHost = document.createElement("td");
    labelColHost.className = "label";
    labelColHost.appendChild(document.createTextNode("Host:"))
    hostRow.appendChild(labelColHost);

    let settingsColHost = document.createElement("td");
    settingsColHost.className = "settings";
    hostRow.appendChild(settingsColHost);

    let inputHost = document.createElement("input");
    inputHost.id = "printername_" + newPrinter;
    inputHost.setAttribute("type", "text");
    settingsColHost.appendChild(inputHost);

    // led row
    let ledRow = document.createElement("tr");
    pTable.appendChild(ledRow);

    let labelColled = document.createElement("td");
    labelColled.className = "label";
    labelColled.appendChild(document.createTextNode("led:"))
    ledRow.appendChild(labelColled);

    let settingsColled = document.createElement("td");
    settingsColled.className = "settings";
    ledRow.appendChild(settingsColled);

    let inputLed = document.createElement("input");
    inputLed.id = "printername_" + newPrinter;
    inputLed.setAttribute("type", "text");
    settingsColled.appendChild(inputLed);

    // add to config site
    if (printers < 5) {
        document.getElementById("printer_container").appendChild(pTable);        
    }

    // creating with vanilla js tables is no fun...
}