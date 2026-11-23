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
uint32_t lastInitMs = 0;
}

void begin() {
    Wire.begin(config::kI2cSdaPin, config::kI2cSclPin);
    ready = bmp.begin(0x76) || bmp.begin(0x77);
    lastInitMs = millis();
    Serial.println(ready ? "BMP280 OK" : "BMP280 not found (check wiring)");
}

void read(Reading& reading) {
    reading.valid = false;
    if (!ready) {
        if (millis() - lastInitMs < config::kSensorRetryMs) return;
        begin();
        if (!ready) return;
    }
    const float t = bmp.readTemperature();
    const float p = bmp.readPressure();   // pascals
    if (!isfinite(t) || !isfinite(p) || t < -40 || t > 85 || p < 30000 || p > 110000) {
        ready = false;
        lastInitMs = millis();
        return;
    }
    reading.indoorTempC = t;
    reading.pressurePa = p;
    reading.valid = true;
}
}  // namespace sensors
