#include <Arduino.h>
#include "blinds.h"

// Disabled for offline servo control.
/*
#include <WiFi.h>
#include <Wire.h>
#include "config.h"
#include "controller.h"
#include "display.h"
#include "pins.h"
#include "secrets.h"
#include "sensors.h"
#include "telemetry.h"
#include "wifi_manager.h"
*/

void setup() {
    Serial.begin(115200);
    delay(500);
    /*
    sensors::begin();
    wifi_manager::begin();
    telemetry::begin();
    */
    blinds::begin();
    Serial.println("Servo ready at 0 (horizontal). Send 0..90 followed by Enter; 90 = vertical.");
}

void loop() {
    /*
    wifi_manager::update();

    static uint32_t lastPublishMs = 0;
    static bool first = true;
    const uint32_t now = millis();

    if (WiFi.status() == WL_CONNECTED &&
        (first || now - lastPublishMs >= config::kTelemetryIntervalMs)) {
        first = false;
        lastPublishMs = now;   // set even on failure to never exceed ThingSpeak's 15 s limit

        telemetry::Sample sample;
        sensors::read(sample.indoor);
        sample.wifiRssiDbm = WiFi.RSSI();

        if (sample.indoor.valid) telemetry::publish(sample);
        else Serial.println("Sensor read failed, skipping upload");
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
