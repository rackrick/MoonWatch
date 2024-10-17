#include "MoonrakerClient.h"
#include <WiFiClient.h>

WiFiClient wifiClient;

using namespace RP;

PrinterStatus MoonrakerClient::getData(String host) {    
    // try get data from moonraker
    http.begin(wifiClient, "http://" + host + "/printer/objects/query?webhooks&virtual_sdcard&print_stats&heater_bed&extruder");    
    int httpCode = http.GET();
    
    // on http OK (200) parse JSON
    if (httpCode == 200) {
        // allocate json buffer      
        DynamicJsonDocument doc(2048);
        String payload = http.getString();
        DeserializationError jsonError = deserializeJson(doc, payload);

        // error on parsing json?
        if (jsonError) {
            // print to serial for debugging
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(jsonError.f_str());

            // return Printer with connectionState "jsonError"
            PrinterStatus errorResult;
            errorResult.connectionState = "jsonError";

            return errorResult;
        }

        // deserialiazation succeed move json to Printer struct
        PrinterStatus result;
        result.connectionState = "success";                                
        result.bedTemp = doc["result"]["status"]["heater_bed"]["temperature"].as<String>().toFloat();
        result.bedTarget = doc["result"]["status"]["heater_bed"]["target"].as<String>().toFloat();
        result.extruderTemp = doc["result"]["status"]["extruder"]["temperature"].as<String>().toFloat();
        result.extruderTarget = doc["result"]["status"]["extruder"]["target"].as<String>().toFloat();
        result.isActive = (doc["result"]["status"]["virtual_sdcard"]["is_active"] == true);
        result.progress = doc["result"]["status"]["virtual_sdcard"]["progress"].as<String>().toFloat();                
        result.state = doc["result"]["status"]["print_stats"]["state"].as<String>();
        result.filename = doc["result"]["status"]["print_stats"]["filename"].as<String>();
        result.printDuration = doc["result"]["status"]["print_stats"]["print_duration"].as<String>().toInt();

        return result;                  
    }
    else {        
        // print to for debugging
        Serial.println("HTTP Error: " + String(httpCode) + " Printer Host: " + String(host));
        
        PrinterStatus httpErrorResult;
        httpErrorResult.connectionState = "Error:" + String(httpCode);

        return httpErrorResult;
    }
}