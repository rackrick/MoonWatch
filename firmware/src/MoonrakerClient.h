#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#ifndef ArduinoJsonH
    #define ArduinoJsonH
    #include <ArduinoJson.h>
#endif

#ifndef PrinterStatusH
    #define PrinterStatusH
    #include "PrinterStatus.h"
#endif

// #include "PrinterStatus.h"

namespace RP
{
    class MoonrakerClient {    

        private:
            HTTPClient http;

        public: 
            PrinterStatus getData(String host);
    };
}

