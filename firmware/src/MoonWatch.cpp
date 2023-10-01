#include "MoonWatch.h"
using namespace RP;

CRGB leds[NUM_LEDS];

///<summary>
/// setup hardware, connection and stuff.
///<summary>
void MoonWatch::setupMonitor() {
    // setup serial
    Serial.begin(9600);

    // setup gpio
    pinMode(LED_BUILTIN, OUTPUT);

    //soft ap
    WiFi.softAP(F("MoonWatch"));

    // filesystem
    Serial.println(F("set up filesystem..."));

    LittleFSConfig fsCfg;
    fsCfg.setAutoFormat(false);
    LittleFS.setConfig(fsCfg);
    LittleFS.begin();

    // config
    Serial.println(F("loading configuration..."));
    ConfigStore& config = ConfigStore::getInstance();
    if (!config.load()) {
        Serial.println( F("error loading config"));
        return;
    }
    StatusLED = config.led.StatusLed;    

    // setup LED
    // default brightness, all LEDs OFF and statusLED to RED for wifi disconnected
    Serial.println(F("set up leds..."));
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(config.led.Brightness);
    FastLED.show();  
    leds[StatusLED] = CRGB::Red;
    FastLED.show();

    // display    
    Serial.println(F("set up display..."));
    lcd = DisplayFactory::getInstance(static_cast<RP::eDisplayMode>(config.general.display)); //TODO: from config    
    String loadMsg[] = {
        "MoonWatch",
        "V0.14"
    };
    lcd->showStatusMsg(loadMsg, 2);
    delay(2000);

    // WebServer
    Serial.println(F("Starting Webserver..."));
    String fsMsg[] = {String(F("Starting Webserver..."))};
    lcd->showStatusMsg(fsMsg, 1);    
    WebServer webServer = WebServer();
  
    // wifi
    Serial.println(F("set up wifi..."));

    if (config.general.wifi == "" && config.general.password == "") {
        String wifiCfgMsg[] = {            
            String(F("please connect to wifi:")),
            String(F("MoonWatch")),
            String(F("and browse to")),
            String(F("http://192.168.4.1")),
            String(F("to setup your MoonWatch"))
        };

        lcd->showStatusMsg(wifiCfgMsg, 5);

        delay(15*60*1000);

        return;
    }

    WiFi.begin(config.general.wifi, config.general.password);

    String wifiMsg[2];
    wifiMsg[0] = F("Connecting WiFi...");
    
    bool flash = false;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);        
        if (!flash) {
            Serial.print(F("."));
            wifiMsg[1] = F(".");

            FastLED.clear();
            FastLED.show();

            flash = true;
        } else {
            Serial.print(F("*"));
            wifiMsg[1] = F("*");

            leds[StatusLED] = CRGB::Yellow;
            FastLED.show();

            flash = false;
        }
        lcd->showStatusMsg(wifiMsg, 2);
    }
    Serial.println(F("Connected!"));

    // setup complete
    String wifiConnected[2];
    wifiConnected[0] = F("Monitor initialized!");
    wifiConnected[1] = WiFi.localIP().toString();
    lcd->showStatusMsg(wifiConnected, 2);

    Serial.println(F("Monitor initialized!"));
    Serial.print(F("IP: "));    
    Serial.print(WiFi.localIP());

    // disable soft ap
    WiFi.softAPdisconnect(true);

    leds[StatusLED] = CRGB::Green;
    FastLED.show();

    delay(2000);
}
///<summary>
/// monitor main loop
///<summary>
void MoonWatch::startMonitor() {

    // control loop - triggers bools after 1,2 or 15 secs.
    // does not enforce execution. When an operation takes longer then the duration
    // the loop will be delayed.
    ulong currentMillis = millis();
    // one second trigger
    if (currentMillis >= loopMillisSec + 1000)  {
        loopSec = true;
        loopMillisSec = currentMillis;
    }
    // 2 seconds trigger
    if  (currentMillis >= loopMillis2Sec + 2000) {
        loop2Sec = true;
        loopMillis2Sec = currentMillis;
    }
    // 15 seconds trigger
    if  (currentMillis >= loopMillis15Sec + 15 * 1000) {
        loop15Sec = true;
        loopMillis15Sec = currentMillis;
    } 

    // get config
    ConfigStore& config = ConfigStore::getInstance();
   
    // without wifi we're fucked up
    if (WiFi.status() == WL_CONNECTED) {

        // cyrcle printers
        if (loop15Sec) {
            switchPrinter(config.printers);
        }
        
        const String printerName = config.printers[activePrinter].Name; 
        MoonrakerClient mrClient;

        // 2 seconds trigger
        // fetch moonraker data        
        if (loop2Sec) {            
            printer = mrClient.getData(config.printers[activePrinter].Host);
        }

        int printerLed = config.printers[activePrinter].Led;

        // 1 seconds trigger
        // update display & leds
        if (loopSec) {            
            // fetching moonraker data successful
            if (printer.connectionState == "success") {
                // status printing        
                if (printer.state == "printing") {
                    
                    // set blue for printing
                    leds[printerLed] = CRGB::Blue;   

                    // display output
                    lcd->showPrinterScreen(printerName, printer);
                } 
                // printing completed
                else if (printer.state == "complete") {  

                    // blink led green
                    if (leds[printerLed]) {
                        leds[printerLed] = CRGB::Black;
                    } else {
                        leds[printerLed] = CRGB::Green;
                    }

                    // display output
                    lcd->showPrinterScreen(printerName, printer);
                }
                // printer is ready
                else if (printer.state == "ready" || printer.state == "standby") {                
                    // set led green for ready    
                    leds[printerLed] = CRGB::Green;
                    // display output
                    lcd->showPrinterScreen(printerName, printer);
                } 

                // printer paused
                else if (printer.state == "paused") {
                    // blink led yellow
                    if (leds[printerLed]) {
                        leds[printerLed] = CRGB::Black;
                    } else {
                        leds[printerLed] = CRGB::Yellow;
                    }
                    // display output
                    lcd->showPrinterScreen(printerName, printer);
                }
                // if we're not printing, paused, completed, standby or ready
                // propably something went wrong -> blink red
                else {
                    if (leds[printerLed]) {
                        leds[printerLed] = CRGB::Black;
                    } else {
                        leds[printerLed] = CRGB::Red;
                    }
                    // display output
                    lcd->showPrinterScreen(printerName, printer);
                }
            } else {
                Serial.println("idle screen: " + config.printers[activePrinter].Name);
                Serial.println(printer.connectionState);
                
                lcd->showIdleScreen();
                leds[printerLed] = CRGB::Black;            
            }        
            FastLED.show();
        }

        
    } else {
        // wifi not connected, display error
        Serial.println("no wifi connection...");
        Serial.println(WiFi.status());
        String msg[] = {            
            String(F("NO WIFI CONNECTION"))
        };
        lcd->showStatusMsg(msg, 1);
        FastLED.clear();
        leds[StatusLED] = CRGB::Red;
        FastLED.show();

        // and turn the soft ap back on 
        WiFi.begin(config.general.wifi, config.general.password);       
        WiFi.softAP(F("MoonWatch"));
    }


    // end of loop reset trigger bits
    loopSec = false;
    loop2Sec = false;
    loop15Sec = false;
    // delay main loop for 250 ms
    delay(250);
}

bool MoonWatch::switchPrinter(std::vector<PrinterConfig> printers) {

    Serial.println(F("Try to switch"));

    uint nextPrinter = activePrinter;

    // with one printer we can't switch
    if (printers.size() == 1) {
        return false;
    }

    nextPrinter = getNextPrinter(printers, activePrinter);
    Serial.println(F("next id"));
    Serial.println(nextPrinter);

    Serial.println("try next printer " + printers[nextPrinter].Name + "(" + nextPrinter + ")");

    MoonrakerClient mrClient;
    printer = mrClient.getData(printers[nextPrinter].Host);

    if (printer.connectionState == "success") {
        Serial.println(F("success, switch printer"));
        activePrinter = nextPrinter;        
        return true;
    } else {
        Serial.println(F("error, printer not reachable. skip."));
        leds[printers[nextPrinter].Led] = CRGB::Black;    
        activePrinter = getNextPrinter(printers, nextPrinter++);

        return false;
    }
}

uint MoonWatch::getNextPrinter(std::vector<PrinterConfig> printers, uint printerId) {

    uint newId = printerId;
    if (printerId < (printers.size() - 1)) {        
        newId = printerId + 1;                
    } else {
        newId = 0;
    }

    return newId;
}

           