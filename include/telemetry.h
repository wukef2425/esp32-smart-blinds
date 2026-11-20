#pragma once
#include <stdint.h>
#include "sensors.h"
#include "weather.h"

namespace telemetry {
struct Sample {
    sensors::Reading indoor;
    weather::Reading outdoor;
    int blindAngleDeg = 0;
    int wifiRssiDbm = 0;
    uint32_t uptimeSec = 0;
    uint32_t sequence = 0;
    uint32_t bootId = 0;
    uint32_t resetReason = 0;
    uint32_t failedUploads = 0;
    uint32_t offlineSlots = 0;
    uint32_t invalidSensorReads = 0;
};
void begin();
bool publish(const Sample& sample);
}
