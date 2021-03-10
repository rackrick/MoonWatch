// include libaries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// include headers with guard
#ifndef IDisplayH
    #define IDisplayH
    #include "Displays/IDisplay.h"
#endif

#ifndef PrinterStatusH
    #define PrinterStatusH
    #include "PrinterStatus.h"
#endif

#ifndef NtpClockH
    #define NtpClockH
    #include "tools/NtpClock.h"
#endif

#ifndef WebServerH
    #define WebServerH
    #include "WebServer/WebServer.h"
#endif

#ifndef ConfigStoreH
    #define ConfigStoreH
    #include "tools/ConfigStore.h"
#endif

// include header files
#include "MoonrakerClient.h"
#include "Displays/DisplayFactory.h"

// define the led number and pin
#define NUM_LEDS 5 //TODO: allocate dynamic
#define DATA_PIN D0

namespace RP
{
    class MoonWatch {   

        private:
            int StatusLED;
            IDisplay* lcd;
            PrinterStatus printer;            

            ulong loopMilis = 0;
            bool loopSec = false;
            ulong loopMillisSec = 0;
            bool loop2Sec = false;
            ulong loopMillis2Sec = 0;
            bool loop15Sec = false; 
            ulong loopMillis15Sec = 0;

            uint activePrinter = 0;

            bool switchPrinter(std::vector<PrinterConfig> printers);

        public:             
            void setupMonitor();
            void startMonitor();
    };
}