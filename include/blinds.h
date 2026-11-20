#pragma once

namespace blinds {
constexpr int kHorizontalServoAngle = 76;
constexpr int kVerticalServoAngle = 180;
constexpr int kMinRelativeAngle = 0;
constexpr int kMaxRelativeAngle = 90;
void begin();
// Calibrated endpoints: 0 = horizontal, 90 = vertical; linear interpolation between.
void setAngle(int angleDeg);
}
