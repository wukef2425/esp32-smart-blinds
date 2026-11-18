#pragma once
#include <Arduino.h>

namespace sensors {
struct Reading {
    float indoorTempC = 0.0f;
    float pressurePa = 0.0f;
    bool valid = false;
};

// Initialize the BMP280. Returns true if the sensor was found.
bool tempBegin();

// Fill `reading` with the latest temperature/pressure.
void tempRead(Reading& reading);
}  // namespace sensors
