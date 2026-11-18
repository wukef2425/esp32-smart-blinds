#include "sensors.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <math.h>
#include "config.h"

namespace sensors {
namespace {
Adafruit_BMP280 bmp;
bool ready = false;
}

void begin() {
    Wire.begin(config::kI2cSdaPin, config::kI2cSclPin);
    ready = bmp.begin(0x76) || bmp.begin(0x77);
    Serial.println(ready ? "BMP280 OK" : "BMP280 not found (check wiring)");
}

void read(Reading& reading) {
    reading.valid = false;
    if (!ready) return;
    const float t = bmp.readTemperature();
    const float p = bmp.readPressure();   // pascals
    if (isnan(t) || isnan(p)) return;
    reading.indoorTempC = t;
    reading.pressurePa = p;
    reading.valid = true;
}
}  // namespace sensors