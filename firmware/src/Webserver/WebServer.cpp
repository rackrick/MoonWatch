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

        // welcome
        // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //     request->send(LittleFS, "/index.html");
        // });

        // include
        server.serveStatic("/include", LittleFS, "/include");              
        server.onNotFound(notFound);
        server.begin();
    };

   

}