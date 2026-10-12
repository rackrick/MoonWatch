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
            int StatusLED = 0;
            IDisplay* lcd = nullptr;
            PrinterStatus printer;            

            // setup finished successfully / wifi credentials present
            bool setupOk = false;
            bool wifiConfigured = true;

            // loop timers (wrap-safe, see startMonitor)
            ulong lastSecTick = 0;
            ulong last2SecTick = 0;
            ulong last15SecTick = 0;

            uint activePrinter = 0;

            // data of the active printer: we keep showing the last good data and only
            // fall back to the clock after several consecutive failed requests.
            bool hasData = false;
            uint8_t failCount = 0;
            bool wasOffline = false;

            // per printer: last seen state, used to detect a cancelled print
            // (printing/paused -> standby without passing through "complete")
            struct PrinterTrack {
                String lastState;
                bool cancelActive = false;
                ulong cancelStart = 0;
                bool online = false;        // last poll of this printer succeeded
            };
            std::vector<PrinterTrack> track;
            void trackState(uint index, const PrinterStatus& status);
            CRGB stateColor(uint index, const String& state, bool phaseOn);

            // blink phase for the LEDs (toggles once per second)
            bool blink = false;

            // wifi dropout handling
            bool wifiWasDown = false;
            bool wifiDownShown = false;
            bool apRestored = false;
            ulong wifiLostSince = 0;
            ulong lastWifiRetry = 0;

            void setLed(int index, const CRGB& color);
            void handleWifiDown(ulong now);
            bool switchPrinter(const std::vector<PrinterConfig>& printers);

        public:             
            void setupMonitor();
            void startMonitor();
    };
}
