#include "MoonrakerClient.h"

using namespace RP;

// keep the request short: a printer on the LAN answers within a few ms.
// A host that is down must not block the main loop (and with it the display) for long.
#define MOONRAKER_TIMEOUT_MS 2000

PrinterStatus MoonrakerClient::getData(const String& host) {
    PrinterStatus result;

    WiFiClient client;
    HTTPClient http;
    http.setReuse(false);
    http.setTimeout(MOONRAKER_TIMEOUT_MS);

    if (!http.begin(client, "http://" + host + "/printer/objects/query?webhooks&virtual_sdcard&print_stats&heater_bed&extruder")) {
        result.connectionState = F("Error:begin");
        return result;
    }

    int httpCode = http.GET();

    if (httpCode != 200) {
        Serial.println("HTTP Error: " + String(httpCode) + " Printer Host: " + host);
        result.connectionState = "Error:" + String(httpCode);
        http.end();
        return result;
    }

    // Only keep the fields we actually use. Without a filter the extruder / print_stats
    // objects can exceed the buffer on some Klipper versions -> sporadic parse errors.
    StaticJsonDocument<384> filter;
    JsonObject fStatus = filter["result"].createNestedObject("status");
    fStatus["heater_bed"]["temperature"] = true;
    fStatus["heater_bed"]["target"] = true;
    fStatus["extruder"]["temperature"] = true;
    fStatus["extruder"]["target"] = true;
    fStatus["virtual_sdcard"]["is_active"] = true;
    fStatus["virtual_sdcard"]["progress"] = true;
    fStatus["print_stats"]["state"] = true;
    fStatus["print_stats"]["filename"] = true;
    fStatus["print_stats"]["print_duration"] = true;

    String payload = http.getString();
    http.end();

    DynamicJsonDocument doc(1024);
    DeserializationError jsonError = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

    if (jsonError) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(jsonError.f_str());
        result.connectionState = F("jsonError");
        return result;
    }

    JsonObject status = doc["result"]["status"].as<JsonObject>();
    if (status.isNull()) {
        result.connectionState = F("jsonError:nostatus");
        return result;
    }

    result.valid = true;
    result.connectionState = F("success");
    result.bedTemp        = status["heater_bed"]["temperature"] | 0.0f;
    result.bedTarget      = status["heater_bed"]["target"] | 0.0f;
    result.extruderTemp   = status["extruder"]["temperature"] | 0.0f;
    result.extruderTarget = status["extruder"]["target"] | 0.0f;
    result.isActive       = status["virtual_sdcard"]["is_active"] | false;
    result.progress       = status["virtual_sdcard"]["progress"] | 0.0f;
    result.state          = String(status["print_stats"]["state"] | "unknown");
    result.filename       = String(status["print_stats"]["filename"] | "");
    result.printDuration  = (int)(status["print_stats"]["print_duration"] | 0.0f);

    return result;
}
