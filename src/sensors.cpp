#include "sensors.h"

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "pins.h"

namespace sensors {

namespace {
Adafruit_BMP280 tempSensor(&Wire);
bool tempOk = false;
}  // namespace

bool tempBegin() {
    tempOk = tempSensor.begin(IoTPins::BMP280_ADDR);
    return tempOk;
}

void tempRead(Reading& reading) {
    reading.indoorTempC = tempSensor.readTemperature();
    reading.pressurePa = tempSensor.readPressure();
    reading.valid = tempOk;
}

}  // namespace sensors
