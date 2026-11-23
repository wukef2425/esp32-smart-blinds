#include "event_log.h"
#include "wifi_manager.h"
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "secrets.h"
#include <time.h>

namespace wifi_manager {
namespace {
uint32_t lastTryMs = 0;
bool wasUp = false;
}

void begin() {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(secrets::kWifiSsid, secrets::kWifiPassword);
    lastTryMs = millis();
    Serial.printf("Wi-Fi: connecting to %s\n", secrets::kWifiSsid);
}

void update() {
    static bool clockLogged = false;
    const time_t utc = time(nullptr);
    if (!clockLogged && utc >= 1767225600) {
        clockLogged = true;
        event_log::write("Clock: synchronized utc=%lld uptime_ms=%lu\n", (long long)utc, millis());
    }
    const bool up = (WiFi.status() == WL_CONNECTED);
    if (up && !wasUp) event_log::write("Wi-Fi: uptime_ms=%lu connected, IP %s\n", millis(), WiFi.localIP().toString().c_str());
    if (!up && wasUp) event_log::write("Wi-Fi: uptime_ms=%lu lost connection\n", millis());
    wasUp = up;

    if (!up && millis() - lastTryMs >= config::kWifiRetryMs) {
        lastTryMs = millis();
        Serial.println("Wi-Fi: retrying");
        WiFi.disconnect();
        WiFi.begin(secrets::kWifiSsid, secrets::kWifiPassword);
    }
}
}  // namespace wifi_manager
