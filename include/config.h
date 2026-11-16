#pragma once
#include <stdint.h>

namespace config {
constexpr int kI2cSdaPin = 21;
constexpr int kI2cSclPin = 22;
constexpr int kServoSignalPin = 13;
constexpr uint32_t kTelemetryIntervalMs = 5UL * 60UL * 1000UL;
}  // namespace config
