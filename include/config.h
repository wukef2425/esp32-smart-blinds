#pragma once
#include <stdint.h>

namespace config {
constexpr int kI2cSdaPin = 21;
constexpr int kI2cSclPin = 22;
constexpr int kServoSignalPin = 13;
constexpr uint32_t kSensorIntervalMs = 1000;
constexpr uint32_t kSensorRetryMs = 10000;
constexpr char kFirmwareVersion[] = "reliability-72h";
constexpr uint32_t kRunDurationMs = 72UL * 60UL * 60UL * 1000UL;
constexpr float kHotTempC = 28.0f;
constexpr float kCoolTempC = 26.0f;
constexpr uint32_t kWifiRetryMs = 10000;
constexpr uint32_t kTelemetryIntervalMs = 5UL * 60UL * 1000UL;
constexpr uint32_t kHttpTimeoutMs = 8000;

constexpr float kLatitude = 33.6846f;
constexpr float kLongitude = -117.8265f;
// Open-Meteo refreshes "current" data about every 15 min; 10 min is plenty.
constexpr uint32_t kWeatherIntervalMs = 10UL * 60UL * 1000UL;
}  // namespace config
