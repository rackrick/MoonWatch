#pragma once
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "PrinterStatus.h"

namespace RP
{
    class MoonrakerClient {    
        public: 
            PrinterStatus getData(const String& host);
    };
}
