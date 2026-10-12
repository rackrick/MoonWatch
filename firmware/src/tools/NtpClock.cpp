#include "NtpClock.h"

// resync interval when we have a valid time / retry interval when NTP failed.
// NTPClient::update() blocks for up to ~1 s, so it must not be called every display update.
#define NTP_RESYNC_MS (30UL * 60UL * 1000UL)
#define NTP_RETRY_MS  (30UL * 1000UL)

namespace RP {
    namespace Tools {
        NtpClock::NtpClock() : timeClient(ntpUDP, "pool.ntp.org", 0)
        {
        }

        NtpClock& NtpClock::getInstance() {
            static NtpClock instance;
            return instance;
        }

        void NtpClock::begin() {
            ConfigStore& config = ConfigStore::getInstance();
            timeClient.setTimeOffset(config.general.utcoffset * 60 * 60);
            timeClient.begin();
            attempted = false;
            maintain();
        }

        // try to (re)sync when due. A failed sync NEVER overwrites a good time.
        void NtpClock::maintain() {
            ulong now = millis();
            ulong interval = synced ? NTP_RESYNC_MS : NTP_RETRY_MS;

            if (attempted && (now - lastAttempt) < interval) {
                return;
            }
            if (WiFi.status() != WL_CONNECTED) {
                return;
            }

            attempted = true;
            lastAttempt = now;

            if (timeClient.forceUpdate()) {
                baseEpoch = timeClient.getEpochTime();
                baseMillis = millis();
                synced = true;
            }
        }

        bool NtpClock::getTm(struct tm& out) {
            maintain();
            if (!synced) {
                return false;
            }
            time_t raw = baseEpoch + (millis() - baseMillis) / 1000;
            gmtime_r(&raw, &out);   // utc offset is already added to baseEpoch
            return true;
        }

        static String two(int v) {
            return (v < 10) ? ("0" + String(v)) : String(v);
        }

        String NtpClock::getDateTimeString() {
            struct tm t;
            if (!getTm(t)) {
                return F("--.--.---- --:--:--");
            }
            return String(t.tm_mday) + "." + String(t.tm_mon + 1) + "." + String(t.tm_year + 1900) + " " +
                   two(t.tm_hour) + ":" + two(t.tm_min) + ":" + two(t.tm_sec);
        }

        String NtpClock::getTimeString() {
            struct tm t;
            if (!getTm(t)) {
                return F("--:--");
            }
            return two(t.tm_hour) + ":" + two(t.tm_min);
        }
    }
}
