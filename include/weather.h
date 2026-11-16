#pragma once

namespace weather {
struct Reading {
    float outdoorTempC = 0.0f;
    bool valid = false;
};
void begin();
void fetch(Reading& reading);
}
