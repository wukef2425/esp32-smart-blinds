#include "wifi_manager.h"

#include <WiFi.h>
#include "secrets.h"

namespace wifi_manager {

bool begin(uint32_t timeoutMs) {
    if (strlen(WIFI_SSID) == 0) {
        Serial.println("WiFi SSID is empty. Edit include/secrets.h first.");
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Connected. IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    Serial.println("WiFi connection failed.");
    return false;
}

void update() {
}

}  // namespace wifi_manager
