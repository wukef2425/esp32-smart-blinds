#pragma once
#include "sensors.h"
#include "weather.h"

namespace telemetry {
struct Sample {
    sensors::Reading indoor;
    weather::Reading outdoor;
    int blindAngleDeg = 0;
    int wifiRssiDbm = 0;
};
void begin();
void publish(const Sample& sample);
}
