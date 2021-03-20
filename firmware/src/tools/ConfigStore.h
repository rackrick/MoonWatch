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
                bool updateGeneral(GeneralConfig config);
                bool updatePrinters(std::vector<PrinterConfig> printers);
                bool updateLed(LedConfig led);
                bool updateWifi(String ssid, String pass);    
                bool save();
                JsonObject getJson();                               

        private:
            // ctor for singleton
            ConfigStore();

            // properties
            StaticJsonDocument<768> doc;
                
    };
}