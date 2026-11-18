#pragma once
#include <Arduino.h>

// Migrated from iot_wifi.h.
namespace wifi_manager {
// Connect to the WiFi network in secrets.h. Returns true once connected,
// false if the SSID is empty or the timeout expires.
bool begin(uint32_t timeoutMs = 15000);

void update();
}  // namespace wifi_manager
