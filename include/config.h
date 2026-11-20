#pragma once
#include <stdint.h>

namespace config {
constexpr int kI2cSdaPin = 21;
constexpr int kI2cSclPin = 22;
constexpr int kServoSignalPin = 13;
constexpr uint32_t kSensorIntervalMs = 1000;
constexpr float kHotTempC = 28.0f;
constexpr float kCoolTempC = 26.0f;
constexpr uint32_t kWifiRetryMs = 10000;
// 20s for vertical slice test
// For the 72-hour run, change back to 5UL * 60UL * 1000UL.
constexpr uint32_t kTelemetryIntervalMs = 20000;

// Fallback location (Irvine, CA) if IP geolocation fails at boot.
// How long weather::begin() waits for Wi-Fi before giving up on geolocation.
constexpr uint32_t kGeoWifiWaitMs = 15000;
constexpr float kLatitude = 33.6846f;
constexpr float kLongitude = -117.8265f;
// Open-Meteo refreshes "current" data about every 15 min; 10 min is plenty.
constexpr uint32_t kWeatherIntervalMs = 10UL * 60UL * 1000UL;
}  // namespace config
