function moonwatch() {
    document.writeln("hello js");
}

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