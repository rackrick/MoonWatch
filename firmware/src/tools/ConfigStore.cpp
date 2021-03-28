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
                general.utcOffset = doc["general"]["utcoffset"].as<int>();

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
            JsonObject json = doc.as<JsonObject>();
            json["general"]["wifi"] = general.wifi;
            json["general"]["password"] = general.password;
            json["general"]["display"] = general.display;
            json["general"]["utcoffset"] = general.utcOffset;

            JsonArray arrPrinters = json["printers"].as<JsonArray>();

            for (int i = 0; i < printers.size(); i++) {
                arrPrinters[i]["name"] = printers[i].Name;
                arrPrinters[i]["host"] = printers[i].Host;
                arrPrinters[i]["led"] = printers[i].Led;
            }

            json["led"]["numleds"] = led.NumLeds;
            json["led"]["statusled"] = led.StatusLed;
            json["led"]["brightness"] = led.Brightness;

            return json;
        }

        bool ConfigStore::save() {
                
            File config = LittleFS.open(CONFIG_FILE, "w");

            if (serializeJson(getJson(), config) > 0) {

                config.close();
                return true;
            } else {
                config.close();
                return false;
            }            
        }

        bool ConfigStore::updateWifi(String ssid, String pass) {
            general.wifi = ssid;
            general.password = pass;

            return true;
        }

        bool ConfigStore::updateGeneral(GeneralConfig newConfig) {
            if (newConfig.wifi != NULL & newConfig.password != NULL) {
                general.wifi = newConfig.wifi;
                general.password = newConfig.password;
            }

            general.display = newConfig.display;
            general.utcOffset = newConfig.utcOffset;

            return true;
        }

        bool ConfigStore::updatePrinters(std::vector<PrinterConfig> newPrinters) {
            printers = newPrinters;

            return true;
        }

        bool ConfigStore::updateLed(LedConfig newLeds) {
            led = newLeds;

            return true;
        }
}