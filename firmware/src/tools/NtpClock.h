#pragma once
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

#include "ConfigStore.h"

namespace RP
{
    namespace Tools
    {
        class NtpClock {
            public:
                // singleton
                static NtpClock& getInstance();
                NtpClock(NtpClock const&) = delete;
                void operator=(NtpClock const&)  = delete;

                String getDateTimeString();
                String getTimeString();
                bool isSynced() const { return synced; }

                // call once WiFi is up (blocking, max. ~1-2 s)
                void begin();

            private:
                NtpClock();

                WiFiUDP ntpUDP;
                NTPClient timeClient;

                bool synced = false;        // we have a valid time
                ulong baseEpoch = 0;        // local epoch (incl. utc offset) at last sync
                ulong baseMillis = 0;       // millis() at last sync
                ulong lastAttempt = 0;      // millis() of last sync attempt
                bool attempted = false;

                void maintain();
                bool getTm(struct tm& out);
        };
    } // namespace Tools
} // namespace  RP
