#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "blinds.h"
#include "config.h"
#include "controller.h"
#include "display.h"
#include "pins.h"
#include "secrets.h"
#include "sensors.h"
#include "telemetry.h"
#include "wifi_manager.h"

void setup() {
    Serial.begin(115200);
    delay(500);
    sensors::begin();
    wifi_manager::begin();
    telemetry::begin();
}

void loop() {
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
}
