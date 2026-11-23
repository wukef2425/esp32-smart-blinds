#include <Arduino.h>
#include "blinds.h"
#include <WiFi.h>
#include "config.h"
#include "weather.h"
#include "wifi_manager.h"
#include "sensors.h"
#include "telemetry.h"
#include "controller.h"
#include <Preferences.h>
#include <esp_system.h>
#include <time.h>
#include "event_log.h"

namespace {
uint32_t bootId = 0;
uint32_t resetReason = 0;
uint32_t invalidSensorReads = 0;
uint32_t failedUploads = 0;
uint32_t offlineSlots = 0;
uint32_t successfulUploads = 0;
}

void setup() {
    Serial.begin(115200);
    delay(500);
    event_log::begin();
    Preferences preferences;
    bootId = esp_random();
    if (preferences.begin("reliability", false)) {
        const uint32_t nextBoot = preferences.getUInt("boot", 0) + 1;
        if (preferences.putUInt("boot", nextBoot) == sizeof(uint32_t)) bootId = nextBoot;
        else Serial.println("Boot: NVS write failed, using random boot ID");
        preferences.end();
    } else Serial.println("Boot: NVS unavailable, using random boot ID");
    resetReason = esp_reset_reason();
    event_log::write("Boot: fw=%s build=%s %s boot=%lu reset=%lu interval_s=%lu\n",
                  config::kFirmwareVersion, __DATE__, __TIME__, (unsigned long)bootId,
                  (unsigned long)resetReason, (unsigned long)(config::kTelemetryIntervalMs / 1000));
    sensors::begin();
    telemetry::begin();
    blinds::begin();
    controller::begin();

    wifi_manager::begin();
}

void loop() {
    wifi_manager::update();

    const uint32_t now = millis();

    static weather::Reading outdoor;
    static sensors::Reading indoor;
    static uint32_t lastSensorMs = 0;
    static bool firstSensor = true;
    static bool sensorWasValid = false;
    if (firstSensor || now - lastSensorMs >= config::kSensorIntervalMs) {
        const bool firstRead = firstSensor;
        firstSensor = false;
        lastSensorMs = now;
        sensors::read(indoor);
        if (!indoor.valid) ++invalidSensorReads;
        if (firstRead || indoor.valid != sensorWasValid) {
            event_log::write("Sensor: valid=%d temp_c=%.2f\n", indoor.valid, indoor.indoorTempC);
            sensorWasValid = indoor.valid;
        }
        controller::update(indoor, outdoor);
    }
    static bool weatherInitialized = false;
    static bool firstWeather = true;
    static uint32_t lastWeatherMs = 0;
    static uint32_t lastWeatherSuccessMs = 0;
    const uint32_t weatherInterval = outdoor.valid ? config::kWeatherIntervalMs : 60000;
    if (outdoor.valid && now - lastWeatherSuccessMs > 2 * config::kWeatherIntervalMs) outdoor.valid = false;

    if (WiFi.status() == WL_CONNECTED && time(nullptr) >= 1767225600 &&
        (firstWeather || now - lastWeatherMs >= weatherInterval)) {
        if (!weatherInitialized) {
            weather::begin();
            weatherInitialized = true;
        }
        firstWeather = false;
        lastWeatherMs = now;
        weather::fetch(outdoor);
        if (outdoor.valid) lastWeatherSuccessMs = millis();
    }

    static uint32_t lastPublishMs = 0;
    static bool first = true;
    static uint32_t sequence = 0;

    // Count scheduled slots even when Wi-Fi is down, so cloud gaps remain explainable.
    if (first || now - lastPublishMs >= config::kTelemetryIntervalMs) {
        const uint32_t slots = first ? 1 : (now - lastPublishMs) / config::kTelemetryIntervalMs;
        sequence += slots;
        if (slots > 1) event_log::write("Upload: missed_slots=%lu\n", (unsigned long)(slots - 1));
        lastPublishMs = first ? now : lastPublishMs + slots * config::kTelemetryIntervalMs;
        first = false;

        telemetry::Sample sample;
        sample.outdoor = outdoor;
        sample.indoor = indoor;
        sample.wifiRssiDbm = WiFi.RSSI();
        sample.blindAngleDeg = blinds::targetAngle();
        sample.uptimeSec = millis() / 1000;
        sample.sequence = sequence;
        sample.bootId = bootId;
        sample.resetReason = resetReason;
        sample.failedUploads = failedUploads;
        sample.offlineSlots = offlineSlots;
        sample.invalidSensorReads = invalidSensorReads;

        if (WiFi.status() != WL_CONNECTED) {
            ++offlineSlots;
            event_log::write("Upload: boot=%lu seq=%lu skipped=wifi_offline\n",
                          (unsigned long)bootId, (unsigned long)sequence);
        } else if (telemetry::publish(sample)) ++successfulUploads;
        else ++failedUploads;
        event_log::write("Run: boot=%lu seq=%lu uptime_s=%lu success=%lu failed=%lu offline=%lu sensor_errors=%lu valid=%d temp_c=%.2f target=%d\n",
                      (unsigned long)bootId, (unsigned long)sequence, (unsigned long)(millis() / 1000),
                      (unsigned long)successfulUploads, (unsigned long)failedUploads,
                      (unsigned long)offlineSlots, (unsigned long)invalidSensorReads,
                      indoor.valid, indoor.indoorTempC, blinds::targetAngle());
    }

    static bool runReported = false;
    if (!runReported && millis() >= config::kRunDurationMs) {
        runReported = true;
        event_log::write("Reliability: 72h elapsed_s=%lu boot=%lu seq=%lu success=%lu failed=%lu offline=%lu sensor_errors=%lu; continuing\n",
                         (unsigned long)(millis() / 1000), (unsigned long)bootId, (unsigned long)sequence,
                         (unsigned long)successfulUploads, (unsigned long)failedUploads,
                         (unsigned long)offlineSlots, (unsigned long)invalidSensorReads);
    }
    event_log::poll();
}
