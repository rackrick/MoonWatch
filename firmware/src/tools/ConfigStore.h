#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>

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

        private:
            // ctor for singleton
            ConfigStore();
    };
}