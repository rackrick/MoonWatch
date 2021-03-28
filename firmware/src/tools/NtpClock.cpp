#include "NtpClock.h"

namespace RP {
    namespace Tools {
        NtpClock::NtpClock()            
        {            
            updateTime();            
        }

        NtpClock& NtpClock::getInstance() {
            static NtpClock instance;
            return instance;
        }

        String NtpClock::getDateTimeString() {            
            // tick tack clock
            tickTime();

            // get time struct
            ptm = gmtime ((time_t *)&lastTime);  
            int day = ptm->tm_mday;            
            int month = ptm->tm_mon+1;            
            int year = ptm->tm_year+1900;  
            int h = ptm->tm_hour;            
            int m = ptm->tm_min;            
            int s = ptm->tm_sec;

            String hh = (h < 10) ? ("0" + String(h)) : String(h);
            String mm = (m < 10) ? ("0" + String(m)) : String(m);
            String ss = (s < 10) ? ("0" + String(s)) : String(s);

            // return datetime string
            String result =                 
                String(day) + "." + 
                String(month) + "." + 
                String(year) + " " + 
                hh + ":" + mm + ":" + ss;

            return result;               
        }

        String NtpClock::getTimeString() {
            // tick tack clock
            tickTime();
                
            // get time struct
            ptm = gmtime ((time_t *)&lastTime);      
            int h = ptm->tm_hour;            
            int m = ptm->tm_min;            
            int s = ptm->tm_sec;            

            String hh = (h < 10) ? ("0" + String(h)) : String(h);
            String mm = (m < 10) ? ("0" + String(m)) : String(m);
            String ss = (s < 10) ? ("0" + String(s)) : String(s);

            // return datetime string
            String result =
                hh + ":" + mm;

            return result;                   
        }
        
        // resync with ntp and store the current seconds since board start
        void NtpClock::updateTime() { 
            if (!ntpSuccess) {
                delay(1000);
            }

            ConfigStore& config = ConfigStore::getInstance();                              
            long utcOffset = config.general.utcOffset;
            WiFiUDP ntpUDP;
            NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffset);
            timeClient.begin();
            ntpSuccess = timeClient.update();

            lastSync = timeClient.getEpochTime();
            lastTime = lastSync;
            secsLastSync = millis() / 1000;
            secsLastTime = secsLastSync;
        }

        void NtpClock::tickTime() {            
            // get current seconds sice board start
            ulong currentSecs = millis() / 1000;

            // ntp sync older then 5 minutes? then resync.
            if (currentSecs - secsLastSync > 300 || !ntpSuccess) {
                updateTime();
            }

            // to get the current time we use the epoch time from the sync
            // add the the passed seconds since then to calculate the new time
            lastTime = lastTime + (currentSecs - secsLastTime);
            secsLastTime = currentSecs;              
        }
    }
}