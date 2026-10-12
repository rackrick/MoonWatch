#include "MoonWatch.h"
using namespace RP;

CRGB leds[NUM_LEDS];

// number of consecutive failed polls (one poll every 2 s) before we give up on the
// printer and show the clock. Short hiccups (wifi, busy Klipper, one lost packet)
// are ridden out by keeping the last known data on screen.
#define MAX_POLL_FAILURES 4

// how long the LED blinks red after a print was cancelled
#define CANCEL_BLINK_MS 60000UL

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

        // make sure we have a display so loop() never touches a null pointer
        lcd = DisplayFactory::getInstance(RP::eDisplayMode::SPI_ST7735);
        String errMsg[] = { String(F("CONFIG ERROR")), String(F("upload filesystem")) };
        lcd->showStatusMsg(errMsg, 2);
        return;
    }
    StatusLED = config.led.StatusLed;    

    // setup LED
    // default brightness, all LEDs OFF and statusLED to RED for wifi disconnected
    Serial.println(F("set up leds..."));
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(config.led.Brightness);
    FastLED.show();  
    setLed(StatusLED, CRGB::Red);
    FastLED.show();

    // display    
    Serial.println(F("set up display..."));
    lcd = DisplayFactory::getInstance(static_cast<RP::eDisplayMode>(config.general.display));
    String loadMsg[] = {
        "MoonWatch",
        "V0.1.6"
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
        // nothing to connect to: loop() keeps the instructions on screen
        // (the async webserver keeps serving the setup page)
        wifiConfigured = false;
        setupOk = true;
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

            setLed(StatusLED, CRGB::Yellow);
            FastLED.show();

            flash = false;
        }
        lcd->showStatusMsg(wifiMsg, 2);
    }
    Serial.println(F("Connected!"));

    // sync the clock once, now that we have a network
    Tools::NtpClock::getInstance().begin();

    // setup complete
    String wifiConnected[2];
    wifiConnected[0] = F("Monitor initialized!");
    wifiConnected[1] = WiFi.localIP().toString();
    lcd->showStatusMsg(wifiConnected, 2);

    Serial.println(F("Monitor initialized!"));
    Serial.print(F("IP: "));    
    Serial.println(WiFi.localIP());

    // disable soft ap
    WiFi.softAPdisconnect(true);

    setLed(StatusLED, CRGB::Green);
    FastLED.show();

    delay(2000);
    setupOk = true;
}

///<summary>
/// remember the last state per printer. A print that goes from printing/paused straight to
/// standby (Klipper's CANCEL_PRINT resets the file) was cancelled -> blink the LED red for a while.
/// A normal end goes through "complete" and is not affected.
///<summary>
void MoonWatch::trackState(uint index, const PrinterStatus& status) {
    if (index >= track.size()) {
        return;
    }
    PrinterTrack& t = track[index];
    t.online = true;

    // cancelled: either Klipper reports "cancelled", or printing/paused jumps straight to standby
    bool wasRunning = (t.lastState == "printing" || t.lastState == "paused");
    bool cancelledNow = (status.state == "cancelled" && t.lastState != "cancelled" && t.lastState.length() > 0);

    if ((wasRunning && status.state == "standby") || cancelledNow) {
        t.cancelActive = true;
        t.cancelStart = millis();
        Serial.println(F("print cancelled"));
    } else if (status.state != "standby" && status.state != "cancelled") {
        // a new print (or any other state) ends the cancel indication
        t.cancelActive = false;
    }
    t.lastState = status.state;
}

///<summary>
/// LED colour for a printer state. phaseOn=false is the "off" half of a blink.
/// Printers that are not on screen use phaseOn=true, so a blinking colour is shown steady
/// instead of freezing in whatever blink phase it had when we switched away.
///<summary>
CRGB MoonWatch::stateColor(uint index, const String& state, bool phaseOn) {
    if (state == "printing") {
        return CRGB::Blue;
    }
    if (state == "complete") {
        return phaseOn ? CRGB(CRGB::Green) : CRGB(CRGB::Black);
    }
    if (state == "paused") {
        return phaseOn ? CRGB(CRGB::Yellow) : CRGB(CRGB::Black);
    }
    if (state == "ready" || state == "standby" || state == "cancelled") {
        PrinterTrack& t = track[index];
        if (t.cancelActive && (millis() - t.cancelStart) >= CANCEL_BLINK_MS) {
            t.cancelActive = false;
        }
        if (t.cancelActive) {
            // print was cancelled: red for CANCEL_BLINK_MS
            return phaseOn ? CRGB(CRGB::Red) : CRGB(CRGB::Black);
        }
        return CRGB(CRGB::Green);
    }
    // anything else (error, shutdown, ...)
    return phaseOn ? CRGB(CRGB::Red) : CRGB(CRGB::Black);
}

///<summary>
/// bounds-checked LED access (a wrong LED index in the config must not corrupt memory)
///<summary>
void MoonWatch::setLed(int index, const CRGB& color) {
    if (index >= 0 && index < NUM_LEDS) {
        leds[index] = color;
    }
}

///<summary>
/// wifi is down. Short dropouts are ridden out silently (last data stays on screen),
/// longer ones show a message ONCE. Reconnect is retried at a sane interval instead of
/// restarting the connection attempt every loop iteration.
///<summary>
void MoonWatch::handleWifiDown(ulong now) {
    ConfigStore& config = ConfigStore::getInstance();

    if (!wifiWasDown) {
        wifiWasDown = true;
        wifiDownShown = false;
        wifiLostSince = now;
        lastWifiRetry = now;
        Serial.println(F("wifi connection lost"));
    }

    if (!wifiDownShown && (now - wifiLostSince) > 5000) {
        String msg[] = { String(F("NO WIFI CONNECTION")) };
        lcd->showStatusMsg(msg, 1);
        FastLED.clear();
        setLed(StatusLED, CRGB::Red);
        FastLED.show();
        wifiDownShown = true;
    }

    // retry every 15 s
    if ((now - lastWifiRetry) > 15000) {
        lastWifiRetry = now;
        WiFi.begin(config.general.wifi, config.general.password);
    }

    // after 30 s bring the setup access point back, so the device stays configurable
    if (!apRestored && (now - wifiLostSince) > 30000) {
        WiFi.softAP(F("MoonWatch"));
        apRestored = true;
    }
}

///<summary>
/// monitor main loop
///<summary>
void MoonWatch::startMonitor() {

    // setup failed (no config): message is already on screen
    if (!setupOk) {
        delay(250);
        return;
    }

    // get config
    ConfigStore& config = ConfigStore::getInstance();

    // first start: no wifi configured yet
    if (!wifiConfigured) {
        // the AP setup page (/save/wifi) only puts the credentials in RAM:
        // persist them and restart so the device connects with the new settings
        if (config.general.wifi != "") {
            Serial.println(F("wifi credentials received, saving and restarting"));
            config.save();
            delay(1000);    // give the webserver time to answer the browser
            ESP.restart();
        }

        String wifiCfgMsg[] = {            
            String(F("please connect to wifi:")),
            String(F("MoonWatch")),
            String(F("and browse to")),
            String(F("http://192.168.4.1")),
            String(F("to setup your MoonWatch"))
        };
        lcd->showStatusMsg(wifiCfgMsg, 5);
        delay(500);
        return;
    }

    if (config.printers.empty()) {
        String msg[] = { String(F("NO PRINTER")), String(F("CONFIGURED")) };
        lcd->showStatusMsg(msg, 2);
        delay(500);
        return;
    }

    // control loop - triggers after 1, 2 or 15 secs.
    // Uses subtraction so it keeps working when millis() wraps (after ~49 days).
    // A blocking operation (HTTP request) only delays the next trigger, it never stacks up.
    ulong now = millis();
    bool loopSec = false, loop2Sec = false, loop15Sec = false;

    if (now - lastSecTick >= 1000)  {
        loopSec = true;
        lastSecTick = now;
    }
    if (now - last2SecTick >= 2000) {
        loop2Sec = true;
        last2SecTick = now;
    }
    if (now - last15SecTick >= 15000) {
        loop15Sec = true;
        last15SecTick = now;
    } 

    if (WiFi.status() != WL_CONNECTED) {
        handleWifiDown(now);
        delay(250);
        return;
    }

    // wifi (back) up
    if (wifiWasDown) {
        wifiWasDown = false;
        Serial.println(F("wifi connection restored"));
        if (apRestored) {
            WiFi.softAPdisconnect(true);
            apRestored = false;
        }
        if (wifiDownShown) {
            wifiDownShown = false;
            FastLED.clear();
        }
        setLed(StatusLED, CRGB::Green);
        FastLED.show();
    }

    if (track.size() != config.printers.size()) {
        track.resize(config.printers.size());
    }

    if (activePrinter >= config.printers.size()) {
        activePrinter = 0;
    }

    // cycle printers
    bool fetched = false;
    if (loop15Sec && config.printers.size() > 1) {
        fetched = switchPrinter(config.printers);
    }

    // copy what we need: the async webserver may replace the config while an HTTP request yields
    struct { String Name; String Host; int Led; } activeCfg;
    activeCfg.Name = config.printers[activePrinter].Name;
    activeCfg.Host = config.printers[activePrinter].Host;
    activeCfg.Led  = config.printers[activePrinter].Led;
    MoonrakerClient mrClient;

    // fetch moonraker data (not again if switchPrinter just fetched fresh data)
    if (loop2Sec && !fetched) {
        PrinterStatus fresh = mrClient.getData(activeCfg.Host);
        if (fresh.valid) {
            printer = fresh;
            hasData = true;
            failCount = 0;
            trackState(activePrinter, fresh);
        } else if (failCount < 255) {
            failCount++;
            Serial.println("poll failed (" + String(failCount) + "): " + activeCfg.Name + " - " + fresh.connectionState);
        }
    }

    bool offline = !hasData || failCount >= MAX_POLL_FAILURES;

    // leds: once per second (also drives the blink phase)
    if (loopSec) {
        blink = !blink;

        if (!offline) {
            if (wasOffline) {
                Serial.println("printer back online: " + activeCfg.Name);
                wasOffline = false;
            }
            setLed(activeCfg.Led, stateColor(activePrinter, printer.state, blink));
        } else {
            if (!wasOffline) {
                Serial.println("printer offline, idle screen: " + activeCfg.Name);
                wasOffline = true;
            }
            track[activePrinter].online = false;
            setLed(activeCfg.Led, CRGB::Black);            
        }        

        // printers that are not on screen: steady colour of their last known state
        for (size_t i = 0; i < config.printers.size() && i < track.size(); i++) {
            if (i == activePrinter) continue;
            setLed(config.printers[i].Led, track[i].online ? stateColor(i, track[i].lastState, true) : CRGB(CRGB::Black));
        }
        FastLED.show();
    }

    // display: every loop iteration. The display only redraws what changed, so this is cheap,
    // and the clock changes right when the second changes instead of up to 250 ms late
    // (a once-per-second trigger on a 250 ms loop made the seconds stutter).
    if (!offline) {
        lcd->showPrinterScreen(activeCfg.Name, printer);
    } else {
        lcd->showIdleScreen();
    }

    // short pause; delay() also lets wifi / webserver run
    delay(50);
}

///<summary>
/// switch to the next reachable printer. Never touches the data of the printer that is
/// currently shown unless the switch succeeds (the old code overwrote it with the data -
/// or the error - of the next printer, which made the display jump to the clock).
///<summary>
bool MoonWatch::switchPrinter(const std::vector<PrinterConfig>& printers) {

    const size_t count = printers.size();
    if (count < 2) {
        return false;
    }

    MoonrakerClient mrClient;

    for (size_t step = 1; step < count; step++) {
        uint candidate = (activePrinter + step) % count;

        PrinterStatus status = mrClient.getData(printers[candidate].Host);
        if (status.valid) {
            Serial.println("switch to printer " + printers[candidate].Name + "(" + String(candidate) + ")");
            activePrinter = candidate;
            trackState(candidate, status);
            printer = status;
            hasData = true;
            failCount = 0;
            return true;
        }

        Serial.println("printer " + printers[candidate].Name + " not reachable, skip");
        track[candidate].online = false;
        setLed(printers[candidate].Led, CRGB::Black);
    }

    return false;
}
