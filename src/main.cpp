#include <Arduino.h>
#include "blinds.h"
#include <WiFi.h>
#include "config.h"
#include "weather.h"
#include "wifi_manager.h"
#include "sensors.h"
#include "telemetry.h"
#include "controller.h"

/*
#include <Wire.h>
#include "display.h"
#include "pins.h"
#include "secrets.h"
*/

void setup() {
    Serial.begin(115200);
    delay(500);
    sensors::begin();
    telemetry::begin();
    blinds::begin();
    controller::begin();
    Serial.println("Servo ready at 0 (horizontal). Send 0..90 followed by Enter; 90 = vertical.");
    wifi_manager::begin();
}

void loop() {
    wifi_manager::update();

    const uint32_t now = millis();

    static weather::Reading outdoor;
    static sensors::Reading indoor;
    static uint32_t lastSensorMs = 0;
    static bool firstSensor = true;
    if (firstSensor || now - lastSensorMs >= config::kSensorIntervalMs) {
        firstSensor = false;
        lastSensorMs = now;
        sensors::read(indoor);
        if (indoor.valid) Serial.printf("Indoor: %.2f C\n", indoor.indoorTempC);
        else Serial.println("Indoor: sensor read failed, holding servo position");
        controller::update(indoor, outdoor);
    }
    static bool weatherInitialized = false;
    static bool firstWeather = true;
    static uint32_t lastWeatherMs = 0;

    if (WiFi.status() == WL_CONNECTED &&
        (firstWeather || now - lastWeatherMs >= config::kWeatherIntervalMs)) {
        if (!weatherInitialized) {
            weather::begin();
            weatherInitialized = true;
        }
        firstWeather = false;
        lastWeatherMs = now;
        weather::fetch(outdoor);
    }

    static uint32_t lastPublishMs = 0;
    static bool first = true;

    // Publish telemetry if Wi-Fi is connected and either it's the first publish or the interval has passed.
    if (WiFi.status() == WL_CONNECTED &&
        (first || now - lastPublishMs >= config::kTelemetryIntervalMs)) {
        first = false;
        lastPublishMs = now;

        telemetry::Sample sample;
        sample.outdoor = outdoor;
        sample.indoor = indoor;
        sample.wifiRssiDbm = WiFi.RSSI();

        if (sample.indoor.valid) telemetry::publish(sample);
        else Serial.println("Sensor read failed, skipping upload");
    }

    /*
    if (outdoor.outdoorTempC > indoor.indoorTempC) {
        // TODO: Close blind to block sunlight and heat.
    }
    else {
        // TODO: Open blind to allow sunlight and heat.
    }
    */

    static int angle = 0;
    static bool hasDigits = false;
    static bool invalid = false;
    static bool negative = false;
    static bool hasSign = false;
    while (Serial.available()) {
        const char ch = Serial.read();
        if (ch == '\r' || ch == '\n') {
            if (hasDigits || invalid || hasSign) {
                const int relativeAngle = negative ? -angle : angle;
                if (hasDigits && !invalid &&
                    relativeAngle >= blinds::kMinRelativeAngle &&
                    relativeAngle <= blinds::kMaxRelativeAngle) {
                    blinds::setAngle(relativeAngle);
                    Serial.printf("Calibrated target: %d degrees\n", relativeAngle);
                } else {
                    Serial.println("Enter an integer from 0 to 90; 0 = horizontal, 90 = vertical.");
                }
            }
            angle = 0;
            hasDigits = false;
            invalid = false;
            negative = false;
            hasSign = false;
        } else if ((ch == '-' || ch == '+') && !hasDigits && !hasSign && !invalid) {
            negative = ch == '-';
            hasSign = true;
        } else if (ch >= '0' && ch <= '9' && !invalid) {
            hasDigits = true;
            angle = angle * 10 + (ch - '0');
            if (angle > 180) invalid = true;
        } else {
            invalid = true;
        }
    }
}
