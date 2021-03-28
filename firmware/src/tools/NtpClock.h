#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef ConfigStoreH
    #define ConfigStoreH
    #include "ConfigStore.h"
#endif

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

                // void init(long utcOffset);
                String getDateTimeString();
                String getTimeString();

            private:
                // ctor for singleton
                NtpClock();

                // properties
                ulong lastSync;
                ulong lastTime;
                ulong secsLastSync;
                ulong secsLastTime;
                struct tm *ptm;
                bool ntpSuccess = false;
                
                // methods
                void updateTime();
                void tickTime();
        };
        
    } // namespace Tools
        
} // namespace  RP
