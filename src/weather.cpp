#include "weather.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include "config.h"

namespace weather {
namespace {
// Defaults to the config location; replaced by IP geolocation in begin() if it succeeds.
float latitude = config::kLatitude;
float longitude = config::kLongitude;
}

// Look up the device's approximate location from its public IP (ip-api.com, free, no key).
// Called once from setup(). Blocks up to kGeoWifiWaitMs waiting for Wi-Fi.
void begin() {
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < config::kGeoWifiWaitMs) {
        delay(100);
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Geo: no Wi-Fi, using default location");
        return;
    }

    // ip-api.com's free tier is HTTP only. A spoofed reply only changes the location.
    WiFiClient client;
    HTTPClient http;
    // getStream() bypasses chunk decoding; request an unchunked response.
    http.useHTTP10(true);
    http.setTimeout(8000);
    if (!http.begin(client, "http://ip-api.com/json/?fields=status,city,lat,lon")) {
        Serial.println("Geo: http.begin failed, using default location");
        return;
    }

    const int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("Geo: HTTP %d, using default location\n", code);
        http.end();
        return;
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, http.getStream());
    http.end();

    if (err || doc["status"] != "success" || !doc["lat"].is<float>() || !doc["lon"].is<float>()) {
        Serial.println("Geo: bad response, using default location");
        return;
    }

    latitude = doc["lat"].as<float>();
    longitude = doc["lon"].as<float>();
    Serial.printf("Geo: %s (%.4f, %.4f)\n", doc["city"] | "?", latitude, longitude);
}

void fetch(Reading& reading) {
    reading.valid = false;

    char url[160];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast"
             "?latitude=%.4f&longitude=%.4f&current=temperature_2m",
             latitude, longitude);

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    // getStream() bypasses chunk decoding; request an unchunked response.
    http.useHTTP10(true);
    http.setTimeout(8000);
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
