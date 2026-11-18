#pragma once

namespace sensors {
struct Reading {
    float indoorTempC = 0.0f;
    float pressurePa = 0.0f;
    bool valid = false;
};
void begin();
void read(Reading& reading);
}
