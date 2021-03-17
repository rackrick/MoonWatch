#include "ConfigStore.h"


#define CONFIG_FILE "/moonwatch.json"

namespace RP {
        // singleton ctor
        ConfigStore::ConfigStore()            
        {      
        }
        
        ConfigStore& ConfigStore::getInstance() {
            static ConfigStore instance;
            return instance;
        }

        bool ConfigStore::load() {

            // check for config file
            if (LittleFS.exists(CONFIG_FILE)) {
                // open readonly
                File config = LittleFS.open(CONFIG_FILE, "r");
                                
                if (!config) {
                    Serial.println(F("error loading file from LittleFs"));
                    return false;                    
                }
                
                DeserializationError docError = deserializeJson(doc, config);

                if (docError) {
                    Serial.println(F("error parsing json"));
                    return false;
                }

                // general config
                general.wifi = doc["general"]["wifi"].as<String>();
                general.password = doc["general"]["password"].as<String>();
                general.display = doc["general"]["display"].as<int>();

                // printers array
                JsonArray arrPrinters = doc["printers"].as<JsonArray>();                   
                for(uint i = 0; i < arrPrinters.size(); i++) {
                    PrinterConfig entry;
                    entry.Host = arrPrinters[i]["host"].as<String>();
                    entry.Name = arrPrinters[i]["name"].as<String>();
                    entry.Led = arrPrinters[i]["led"].as<int>();
                    printers.push_back(entry);
                }

                // led config
                led.NumLeds = doc["led"]["numleds"].as<int>();                
                led.Brightness = doc["led"]["brightness"].as<int>();
                led.StatusLed = doc["led"]["statusled"].as<int>();

                config.close();

                return true;
            }

            return false;
        }

        JsonObject ConfigStore::getJson() {
            return doc.as<JsonObject>();
        }

        void ConfigStore::save() {

            if (LittleFS.exists(CONFIG_FILE)) {
                LittleFS.remove(CONFIG_FILE);
            }
                
            File config = LittleFS.open(CONFIG_FILE, "w");
            serializeJson(getJson(), config);
            config.close();
        }

        bool ConfigStore::updateWifi(String ssid, String pass) {
            general.wifi = ssid;
            general.password = pass;

            return true;
        }


}