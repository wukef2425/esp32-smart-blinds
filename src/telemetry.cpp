#include "event_log.h"
#include "telemetry.h"
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "config.h"
#include "network_security.h"

namespace telemetry {

void begin() {}

bool publish(const Sample& sample) {
    static bool attempted = false;
    static uint32_t lastAttemptFinishedMs = 0;
    if (attempted && millis() - lastAttemptFinishedMs < 16000) {
        event_log::write("ThingSpeak: skipped=local_rate_guard");
        return false;
    }
    if (strlen(secrets::kThingSpeakWriteApiKey) == 0) {
        event_log::write("ThingSpeak: missing write API key");
        return false;
    }
    WiFiClientSecure client;
    if (!network_security::configure(client)) {
        event_log::write("ThingSpeak: waiting for clock sync");
        return false;
    }

    HTTPClient http;
    http.setConnectTimeout(config::kHttpTimeoutMs);
    http.setTimeout(config::kHttpTimeoutMs);
    if (!http.begin(client, "https://api.thingspeak.com/update")) {
        event_log::write("ThingSpeak: http.begin failed");
        return false;
    }
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    doc["api_key"] = secrets::kThingSpeakWriteApiKey;
    if (sample.indoor.valid) {
        doc["field1"] = sample.indoor.indoorTempC;
        doc["field2"] = sample.indoor.pressurePa / 100.0f;
    }
    doc["field3"] = sample.wifiRssiDbm;
    char status[240];
    snprintf(status, sizeof(status),
             "fw=%s boot=%lu seq=%lu uptime=%lu reset=%lu target=%d outdoor=%.1f weather_valid=%d failed=%lu offline=%lu sensor_errors=%lu indoor_valid=%d",
             config::kFirmwareVersion, (unsigned long)sample.bootId,
             (unsigned long)sample.sequence, (unsigned long)sample.uptimeSec,
             (unsigned long)sample.resetReason, sample.blindAngleDeg,
             sample.outdoor.valid ? sample.outdoor.outdoorTempC : 0.0f, sample.outdoor.valid,
             (unsigned long)sample.failedUploads, (unsigned long)sample.offlineSlots,
             (unsigned long)sample.invalidSensorReads, sample.indoor.valid);
    doc["status"] = status;
    String body;
    serializeJson(doc, body);

    const int code = http.POST(body);
    if (code < 0) {
        char error[128] = {};
        const int tlsError = client.lastError(error, sizeof(error));
        event_log::write("ThingSpeak: transport=%s tls=%d %s\n",
                      HTTPClient::errorToString(code).c_str(), tlsError, error);
    }
    String resp = (code > 0) ? http.getString() : String("");
    http.end();
    attempted = true;
    lastAttemptFinishedMs = millis();

    resp.trim();
    char* end = nullptr;
    const unsigned long entry = strtoul(resp.c_str(), &end, 10);
    const bool accepted = code == HTTP_CODE_OK && entry > 0 &&
                          end != resp.c_str() && *end == '\0';
    event_log::write("ThingSpeak: HTTP %d entry=%lu accepted=%d boot=%lu seq=%lu uptime_s=%lu\n",
                  code, entry, accepted, (unsigned long)sample.bootId,
                  (unsigned long)sample.sequence, (unsigned long)sample.uptimeSec);
    return accepted;
}
}  // namespace telemetry
