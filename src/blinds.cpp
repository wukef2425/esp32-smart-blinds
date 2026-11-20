#include "blinds.h"
#include <ESP32Servo.h>
#include "config.h"

namespace blinds {
namespace {
Servo servo;
}

void begin() {
    servo.setPeriodHertz(50);
    servo.attach(config::kServoSignalPin, 600, 2400);
    setAngle(0);
}
void setAngle(int angleDeg) {
    const int relativeAngle = constrain(angleDeg, kMinRelativeAngle, kMaxRelativeAngle);
    servo.write(map(relativeAngle, kMinRelativeAngle, kMaxRelativeAngle,
                    kHorizontalServoAngle, kVerticalServoAngle));
}
}
