#include "blinds.h"
#include <ESP32Servo.h>
#include "config.h"

namespace blinds {
namespace {
Servo servo;
int target = 0;
}

void begin() {
    servo.setPeriodHertz(50);
    servo.attach(config::kServoSignalPin, 600, 2400);
    setAngle(0);
}
void setAngle(int angleDeg) {
    const int relativeAngle = constrain(angleDeg, kMinRelativeAngle, kMaxRelativeAngle);
    target = relativeAngle;
    servo.write(map(relativeAngle, kMinRelativeAngle, kMaxRelativeAngle,
                    kHorizontalServoAngle, kVerticalServoAngle));
}
int targetAngle() {
    return target;
}
}
