#include "weather.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include "config.h"
#include "network_security.h"

namespace weather {
namespace {
// Defaults to the config location; replaced by IP geolocation in begin() if it succeeds.
float latitude = config::kLatitude;
float longitude = config::kLongitude;
}

void begin() {
    Serial.printf("Weather: fixed location %.4f, %.4f\n", latitude, longitude);
}

void fetch(Reading& reading) {
    reading.valid = false;

    char url[160];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast"
             "?latitude=%.4f&longitude=%.4f&current=temperature_2m",
             latitude, longitude);

    WiFiClientSecure client;
    if (!network_security::configure(client)) {
        Serial.println("Weather: waiting for clock sync");
        return;
    }

    HTTPClient http;
    // getStream() bypasses chunk decoding; request an unchunked response.
    http.useHTTP10(true);
    http.setConnectTimeout(config::kHttpTimeoutMs);
    http.setTimeout(config::kHttpTimeoutMs);
    if (!http.begin(client, url)) {
        Serial.println("Weather: http.begin failed");
        return;
    }

    const int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("Weather: HTTP %d\n", code);
        http.end();
        return;
    }

    // Only keep current.temperature_2m to save RAM.
    JsonDocument filter;
    filter["current"]["temperature_2m"] = true;

    JsonDocument doc;
    const DeserializationError err =
        deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end();

    if (err) {
        Serial.printf("Weather: JSON error %s\n", err.c_str());
        return;
    }

    JsonVariantConst temp = doc["current"]["temperature_2m"];
    if (!temp.is<float>()) {
        Serial.print("Weather: missing or nonnumeric temperature_2m, parsed: ");
        serializeJson(doc, Serial);
        Serial.println();
        return;
    }

    const float temperature = temp.as<float>();
    if (!isfinite(temperature)) {
        Serial.println("Weather: invalid temperature_2m");
        return;
    }
    reading.outdoorTempC = temperature;
    reading.valid = true;
    Serial.printf("Weather: outdoor %.1f C\n", reading.outdoorTempC);
}
}  // namespace weather
