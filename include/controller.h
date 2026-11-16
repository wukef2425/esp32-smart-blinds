#pragma once
#include "sensors.h"
#include "weather.h"

namespace controller {
void begin();
void update(const sensors::Reading& indoor, const weather::Reading& outdoor);
}
