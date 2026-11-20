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
#include "weather.h"
#include "wifi_manager.h"

void setup() {
    Serial.begin(115200);
    delay(500);
    sensors::begin();
    wifi_manager::begin();
    telemetry::begin();
    weather::begin();
}

void loop() {
    wifi_manager::update();

    static uint32_t lastPublishMs = 0;
    static bool first = true;
    const uint32_t now = millis();

    static weather::Reading outdoor;
    static sensors::Reading indoor;
    static bool firstWeather = true;
    static uint32_t lastWeatherMs = 0;
    sensors::read(indoor);

    // Fetch weather data if Wi-Fi is connected and either it's the first fetch or the interval has passed.
    if (WiFi.status() == WL_CONNECTED &&
        (firstWeather || now - lastWeatherMs >= config::kWeatherIntervalMs)) {
        firstWeather = false;
        lastWeatherMs = now;
        weather::fetch(outdoor);
    }

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

    if (outdoor.outdoorTempC > indoor.indoorTempC) {
        // TODO: Close blind to block sunlight and heat.
    }
    else {
        // TODO: Open blind to allow sunlight and heat.
    }
}
