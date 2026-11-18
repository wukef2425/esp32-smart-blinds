#pragma once
#include <stdint.h>

namespace config {
constexpr int kI2cSdaPin = 21;
constexpr int kI2cSclPin = 22;
constexpr int kServoSignalPin = 13;
constexpr uint32_t kWifiRetryMs = 10000;
// Slice demo: 20 s so a TA sees updates quickly (ThingSpeak minimum is 15 s).
// For the 72-hour run, change back to 5UL * 60UL * 1000UL.
constexpr uint32_t kTelemetryIntervalMs = 20000;
}  // namespace config