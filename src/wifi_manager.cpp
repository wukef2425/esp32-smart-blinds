#include "wifi_manager.h"
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "secrets.h"

#include <WiFi.h>
#include "secrets.h"

namespace wifi_manager {
namespace {
uint32_t lastTryMs = 0;
bool wasUp = false;
}

void begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(secrets::kWifiSsid, secrets::kWifiPassword);
    lastTryMs = millis();
    Serial.printf("Wi-Fi: connecting to %s\n", secrets::kWifiSsid);
}

void update() {
    const bool up = (WiFi.status() == WL_CONNECTED);
    if (up && !wasUp) Serial.printf("Wi-Fi: connected, IP %s\n", WiFi.localIP().toString().c_str());
    if (!up && wasUp) Serial.println("Wi-Fi: lost connection");
    wasUp = up;

    if (!up && millis() - lastTryMs >= config::kWifiRetryMs) {
        lastTryMs = millis();
        Serial.println("Wi-Fi: retrying");
        WiFi.disconnect();
        WiFi.begin(secrets::kWifiSsid, secrets::kWifiPassword);
    }
}
}  // namespace wifi_manager
