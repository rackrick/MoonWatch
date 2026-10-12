/**
 * MoonWatch configuration UI
 * Fixed printer counter, missing storewifi(), cleaner ID handling
 */

const MAX_PRINTERS = 5;

/**
 * Load current config from the device and fill the form
 */
function load() {
    fetch("/config")
        .then(response => {
            if (!response.ok) throw new Error("HTTP " + response.status);
            return response.json();
        })
        .then(data => updateConfig(data))
        .catch(err => {
            console.error("Failed to load config:", err);
            alert("Could not load configuration.");
        });
}

/**
 * Fill the form with the received config data
 */
function updateConfig(data) {
    // General
    if (document.getElementById("wifi")) {
        document.getElementById("wifi").value = data.general.wifi || "";
    }
    if (document.getElementById("display")) {
        document.getElementById("display").value = data.general.display;
    }
    if (document.getElementById("utcoffset")) {
        document.getElementById("utcoffset").value = data.general.utcoffset;
    }

    // Clear existing printer rows (in case load is called again)
    const container = document.getElementById("printer_container");
    if (container) {
        container.innerHTML = "";
    }

    // Printers
    if (data.printers && Array.isArray(data.printers)) {
        data.printers.forEach(p => {
            addPrinter(p.name, p.host, p.led);
        });
    }

    // LED settings
    if (document.getElementById("numleds")) {
        document.getElementById("numleds").value = data.led.numleds;
    }
    if (document.getElementById("statusled")) {
        document.getElementById("statusled").value = data.led.statusled;
    }
    if (document.getElementById("brightness")) {
        document.getElementById("brightness").value = data.led.brightness;
    }
}

/**
 * Save the complete configuration (used on the normal config page)
 */
function save() {
    const confObj = {
        general: {
            wifi: document.getElementById("wifi").value,
            password: document.getElementById("password").value,
            display: document.getElementById("display").value,
            utcoffset: document.getElementById("utcoffset").value
        },
        printers: [],
        led: {
            numleds: document.getElementById("numleds").value,
            brightness: document.getElementById("brightness").value,
            statusled: document.getElementById("statusled").value
        }
    };

    const container = document.getElementById("printer_container");
    const tables = container ? container.querySelectorAll("table.tblPrinter") : [];

    tables.forEach((table, index) => {
        const nameEl = document.getElementById("printername_" + index);
        const hostEl = document.getElementById("printerhost_" + index);
        const ledEl  = document.getElementById("printerled_" + index);

        if (nameEl && hostEl && ledEl) {
            confObj.printers.push({
                name: nameEl.value.trim(),
                host: hostEl.value.trim(),
                led:  parseInt(ledEl.value, 10) || 0
            });
        }
    });

    const options = {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(confObj)
    };

    fetch("/config", options)
        .then(response => response.json())
        .then(data => resetEsp(data))
        .catch(err => {
            console.error("Save failed:", err);
            alert("Save failed. Check console for details.");
        });
}

/**
 * Called after a successful save – shows a message
 * (actual reboot is left to the user, as the original code did)
 */
function resetEsp(data) {
    if (data && data.status === "ok") {
        alert("Settings saved.\nPlease restart your MoonWatch to apply the new settings.");
    } else {
        alert("Something went wrong while saving.");
    }
}

/**
 * Add a new printer row to the form.
 * IDs are always sequential based on the current number of rows.
 */
function addPrinter(name, host, led) {
    const container = document.getElementById("printer_container");
    if (!container) return;

    const currentCount = container.querySelectorAll("table.tblPrinter").length;

    if (currentCount >= MAX_PRINTERS) {
        alert("Maximum of " + MAX_PRINTERS + " printers reached.");
        return;
    }

    const id = currentCount;   // 0, 1, 2, ...

    // Create table
    const pTable = document.createElement("table");
    pTable.id = "printer_" + id;
    pTable.className = "tblPrinter";

    // --- Name row ---
    const nameRow = document.createElement("tr");
    const labelName = document.createElement("td");
    labelName.className = "label";
    labelName.textContent = "Printer Name:";
    const settingsName = document.createElement("td");
    settingsName.className = "settings";
    const inputName = document.createElement("input");
    inputName.type = "text";
    inputName.id = "printername_" + id;
    inputName.placeholder = "My Printer";
    if (name !== undefined && name !== null) inputName.value = name;
    settingsName.appendChild(inputName);
    nameRow.appendChild(labelName);
    nameRow.appendChild(settingsName);
    pTable.appendChild(nameRow);

    // --- Host row ---
    const hostRow = document.createElement("tr");
    const labelHost = document.createElement("td");
    labelHost.className = "label";
    labelHost.textContent = "Host:";
    const settingsHost = document.createElement("td");
    settingsHost.className = "settings";
    const inputHost = document.createElement("input");
    inputHost.type = "text";
    inputHost.id = "printerhost_" + id;
    inputHost.placeholder = "192.168.1.50 or hostname";
    if (host !== undefined && host !== null) inputHost.value = host;
    settingsHost.appendChild(inputHost);
    hostRow.appendChild(labelHost);
    hostRow.appendChild(settingsHost);
    pTable.appendChild(hostRow);

    // --- LED row ---
    const ledRow = document.createElement("tr");
    const labelLed = document.createElement("td");
    labelLed.className = "label";
    labelLed.textContent = "Led#:";
    const settingsLed = document.createElement("td");
    settingsLed.className = "settings";
    const inputLed = document.createElement("input");
    inputLed.type = "number";
    inputLed.id = "printerled_" + id;
    inputLed.min = "0";
    inputLed.max = "4";
    inputLed.placeholder = "0-4";
    if (led !== undefined && led !== null) inputLed.value = led;
    settingsLed.appendChild(inputLed);
    ledRow.appendChild(labelLed);
    ledRow.appendChild(settingsLed);
    pTable.appendChild(ledRow);

    container.appendChild(pTable);
}

/**
 * Remove the last printer row
 */
function delPrinter() {
    const container = document.getElementById("printer_container");
    if (!container) return;

    const tables = container.querySelectorAll("table.tblPrinter");
    if (tables.length === 0) {
        alert("No printers to remove.");
        return;
    }

    if (confirm("Remove the last printer?")) {
        tables[tables.length - 1].remove();
        // Re-number remaining printers so IDs stay sequential (0,1,2…)
        renumberPrinters();
    }
}

/**
 * After a delete, re-assign sequential IDs to the remaining rows.
 * This keeps save() simple and reliable.
 */
function renumberPrinters() {
    const container = document.getElementById("printer_container");
    if (!container) return;

    const tables = container.querySelectorAll("table.tblPrinter");
    tables.forEach((table, index) => {
        table.id = "printer_" + index;

        const nameInput = table.querySelector("input[id^='printername_']");
        const hostInput = table.querySelector("input[id^='printerhost_']");
        const ledInput  = table.querySelector("input[id^='printerled_']");

        if (nameInput) nameInput.id = "printername_" + index;
        if (hostInput) hostInput.id = "printerhost_" + index;
        if (ledInput)  ledInput.id  = "printerled_"  + index;
    });
}

/**
 * Used on the Soft-AP setup page (index-ap.html)
 * Saves only WiFi credentials.
 */
function storewifi() {
    const ssid = document.getElementById("wifi").value.trim();
    const pass = document.getElementById("password").value;

    if (!ssid) {
        alert("Please enter a WiFi name (SSID).");
        return;
    }

    const payload = {
        wifi: ssid,
        password: pass
    };

    fetch("/save/wifi", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    })
    .then(response => response.json())
    .then(data => {
        if (data && data.status === "ok") {
            alert("WiFi settings saved.\nThe device will now try to connect.\nYou can close this page.");
        } else {
            alert("Failed to save WiFi settings.");
        }
    })
    .catch(err => {
        console.error("storewifi error:", err);
        alert("Error while saving WiFi settings.");
    });
}
