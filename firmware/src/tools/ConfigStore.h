#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include <string.h>

#include "GeneralConfig.h"
#include "PrinterConfig.h"
#include "LedConfig.h"

namespace RP {
    class ConfigStore {
        public:
                // singleton
                static ConfigStore& getInstance();
                ConfigStore(ConfigStore const&) = delete;
                void operator=(ConfigStore const&)  = delete;

                // properties                
                GeneralConfig general;
                std::vector<PrinterConfig> printers;
                LedConfig led;                                

                // methods
                bool load();   
                bool updateWifi(String ssid, String pass);    
                JsonObject getJson();                               

        private:
            // ctor for singleton
            ConfigStore();

            // properties
            StaticJsonDocument<768> doc;

            // methods
            void save();
    };
}