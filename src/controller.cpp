#include "event_log.h"
#include "controller.h"
#include <Arduino.h>
#include <math.h>
#include "blinds.h"
#include "config.h"

namespace controller {
namespace {
bool hot = false;
}
void begin() {
    hot = false;
    blinds::setAngle(0);
    event_log::write("Auto: >= %.1f C -> 90, <= %.1f C -> 0\n",
                  config::kHotTempC, config::kCoolTempC);
}
void update(const sensors::Reading& indoor, const weather::Reading& /*outdoor*/) {
    if (!indoor.valid || !isfinite(indoor.indoorTempC)) return;
    bool nextHot = hot;
    if (indoor.indoorTempC >= config::kHotTempC) nextHot = true;
    else if (indoor.indoorTempC <= config::kCoolTempC) nextHot = false;
    if (nextHot == hot) return;
    hot = nextHot;
    const int angle = hot ? 90 : 0;
    blinds::setAngle(angle);
    event_log::write("Auto: %.2f C -> %d degrees\n", indoor.indoorTempC, angle);
}
}
