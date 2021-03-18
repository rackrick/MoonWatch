#include "WebServer.h"

AsyncWebServer server(80);

namespace RP {
    void notFound(AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    }


    WebServer::WebServer()
    {
        // home
        server.rewrite("/", "index.html");
        server.rewrite("/index.html", "index-ap.html").setFilter(ON_AP_FILTER);  
        server.serveStatic("/", LittleFS, "/");

        // wifi
        AsyncCallbackJsonWebHandler* wifiEndpoint = new AsyncCallbackJsonWebHandler("/save/wifi", [](AsyncWebServerRequest *request, JsonVariant &json) {
            
            if (json.size() > 0) {
                
                JsonObject jsonObj = json.as<JsonObject>();

                String ssid = jsonObj["wifi"];
                String pass = jsonObj["password"];
                
                if (ssid != NULL && pass != NULL) {
                    ConfigStore& config = ConfigStore::getInstance();
                    
                    if (config.updateWifi(ssid, pass)) {
                        request->send(200, "application/json", "{ \"status\": \"ok\" }");
                    } else {
                        request->send(200, "application/json", "{ \"status\": \"error\" }");
                    }
                } else {
                    request->send(200, "application/json", "{ \"status\": \"error\" }");
                }                
            } else {
                request->send(200, "application/json", "{ \"status\": \"error\" }");
            }
        });
        server.addHandler(wifiEndpoint);

        // get complete config, without password
        server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){

            AsyncJsonResponse * response = new AsyncJsonResponse();
            JsonVariant& root = response->getRoot();

            ConfigStore& config = ConfigStore::getInstance();                        
            JsonObject configJson = config.getJson();
            
            // remove pw
            configJson["general"]["password"] = "";
            
            // prepare response
            root.set(configJson);
            response->setLength();

            request->send(response);
        });

        // post complete config, to save
        AsyncCallbackJsonWebHandler* configEndpoint = new AsyncCallbackJsonWebHandler("/config", [](AsyncWebServerRequest *request, JsonVariant &json) {
            
            if (json.size() > 0) {
                
                // get config singleton
                ConfigStore& configStore = ConfigStore::getInstance();

                // convert json
                JsonObject jsonObj = json.as<JsonObject>();

                // write general config                    
                GeneralConfig config;                
                // skip wifi settings on empty password
                if (jsonObj["general"]["password"].as<String>() != "") {
                    config.wifi = jsonObj["general"]["wifi"].as<String>();
                    config.password = jsonObj["general"]["password"].as<String>();
                    config.display = jsonObj["general"]["display"].as<int>();
                } else {
                    config.display = jsonObj["general"]["display"].as<int>();
                }

                bool updateGeneral = configStore.updateGeneral(config);

                // update printers
                JsonArray arrPrinters = jsonObj["printers"].as<JsonArray>();
                std::vector<PrinterConfig> newPrinters;

                for (int i = 0; i < arrPrinters.size(); i++) {                    
                    PrinterConfig helperPrinter;
                    helperPrinter.Name = arrPrinters[i]["name"].as<String>();
                    helperPrinter.Host = arrPrinters[i]["host"].as<String>();
                    helperPrinter.Led = arrPrinters[i]["led"].as<int>();

                    newPrinters.push_back(helperPrinter);
                }

                bool updatePrinters = configStore.updatePrinters(newPrinters);

                // update led settings
                LedConfig newLed;
                newLed.NumLeds = jsonObj["led"]["numleds"].as<int>();
                newLed.StatusLed = jsonObj["led"]["statusled"].as<int>();
                newLed.Brightness = jsonObj["led"]["brightness"].as<int>();

                bool updateLeds = configStore.updateLed(newLed);

                if (updateGeneral && updatePrinters && updateLeds) {
                    Serial.println(F("saving config"));
                    Serial.println(configStore.save());
                    request->send(200, "application/json", "{ \"status\": \"ok\" }");
                } else {
                    request->send(200, "application/json", "{ \"status\": \"error\" }");
                }

            } else {
                request->send(200, "application/json", "{ \"status\": \"error\" }");
            }
        });
        server.addHandler(configEndpoint);

        // reset esp
        server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
            Serial.println(F("reset called"));
            ESP.reset();
        });

        // include        
        server.serveStatic("/include", LittleFS, "/include");              
        server.onNotFound(notFound);
        server.begin();
    };
}