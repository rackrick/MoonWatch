#include "WebServer.h"

AsyncWebServer server(80);

namespace RP {
    void notFound(AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    }


    WebServer::WebServer()
    {
        // welcome
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/index.html");
        });

        // include
        server.serveStatic("/include", LittleFS, "/include");        
        server.onNotFound(notFound);
        server.begin();
    };

   

}