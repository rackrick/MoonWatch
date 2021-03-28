let printers = 0;

// load settings
function load() {
    fetch("/config")
    .then(response => response.json())
    .then(data =>  updateConfig(data));    
}

// display settings und site
function updateConfig(data) {
    // general settings
    document.getElementById("wifi").value = data.general.wifi;
    document.getElementById("display").value = data.general.display;
    document.getElementById("utcOffset").value = data.general.utcOffset;

    // printers
    for (let i = 0; i < data.printers.length; i++) {
        addPrinter(
            data.printers[i].name,
            data.printers[i].host,
            data.printers[i].led
        );
    }

    document.getElementById("numleds").value = data.led.numleds;
    document.getElementById("statusled").value = data.led.statusled;
    document.getElementById("brightness").value = data.led.brightness;
}


// store wifi 
function save() {

    confObj = {
        general: {
            wifi: document.getElementById("wifi").value,
            password: document.getElementById("password").value,
            display: document.getElementById("display").value,
            utcOffset: document.getElementById("utcOffset").value,
        }, 
        printers: [],
        led: {
            numleds: document.getElementById("numleds").value,
            brightness: document.getElementById("brightness").value,
            statusled: document.getElementById("statusled").value
        }        
    }

    let printerTbl = document.getElementById("printer_container");

    for (let i = 0; i < printerTbl.childNodes.length; i++) {
        printerObj = {
            name: document.getElementById("printername_"+ i).value,
            host: document.getElementById("printerhost_"+ i).value,
            led: document.getElementById("printerled_" + i).value
        }
        confObj.printers.push(printerObj);
    }

    let options = {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(confObj)
    }

    fetch("/config", options)
    .then(response => response.json())
    .then(data => resetEsp(data));
}

function resetEsp(data) {
    if (data.status == "ok") {
        alert("settings saved. restart your MoonWatch to apply the new settings.")
    }
}

// add a new printer printer inputs to config site
function addPrinter(name, host, led) {

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
    
    if (name !== undefined) {
        inputName.value = name;
    }
    
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
    inputHost.id = "printerhost_" + newPrinter;
    inputHost.setAttribute("type", "text");

    if (host !== undefined) {
        inputHost.value = host;
    }

    settingsColHost.appendChild(inputHost);

    // led row
    let ledRow = document.createElement("tr");
    pTable.appendChild(ledRow);

    let labelColled = document.createElement("td");
    labelColled.className = "label";
    labelColled.appendChild(document.createTextNode("Led#:"))
    ledRow.appendChild(labelColled);

    let settingsColled = document.createElement("td");
    settingsColled.className = "settings";
    ledRow.appendChild(settingsColled);

    let inputLed = document.createElement("input");
    inputLed.id = "printerled_" + newPrinter;
    inputLed.setAttribute("type", "text");

    if (led !== undefined) {
        inputLed.value = led;
    }

    settingsColled.appendChild(inputLed);

    // add to config site
    if (printers < 5) {
        document.getElementById("printer_container").appendChild(pTable);        
    }

    // creating with vanilla js tables is no fun...
}

// remove the last printer
function delPrinter() {
    let tblPrinters = document.getElementById("printer_container");
    if (confirm("remove printer?"))
    tblPrinters.childNodes[tblPrinters.childNodes.length - 1].remove();
}