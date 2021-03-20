#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

#ifndef ConfigStoreH
    #define ConfigStoreH
    #include "tools/ConfigStore.h"
#endif

namespace RP {
    class WebServer {
        public:
            WebServer();

    };
}
