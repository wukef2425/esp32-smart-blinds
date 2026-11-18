#include "telemetry.h"
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "secrets.h"

namespace telemetry {

void begin() {}

// WARNING: setInsecure() encrypts the connection but does NOT verify the server.
// Someone on a fake hotspot could impersonate ThingSpeak and read our write key.
// Fine for the vertical slice; pin the root certificate later.
void publish(const Sample& sample) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(8000);
    if (!http.begin(client, "https://api.thingspeak.com/update")) {
        Serial.println("ThingSpeak: http.begin failed");
        return;
    }
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    char body[160];
    snprintf(body, sizeof(body), "api_key=%s&field1=%.2f&field2=%.1f&field3=%d",
             secrets::kThingSpeakWriteApiKey,
             sample.indoor.indoorTempC,
             sample.indoor.pressurePa / 100.0f,   // Pa -> hPa
             sample.wifiRssiDbm);

    const int code = http.POST((uint8_t*)body, strlen(body));
    const String resp = (code > 0) ? http.getString() : String("");
    http.end();

    // ThingSpeak replies with the new entry number, or "0" if it rejected the update.
    Serial.printf("ThingSpeak: HTTP %d, entry %s  (T=%.2f C, P=%.1f hPa, RSSI=%d)\n",
                  code, resp.c_str(), sample.indoor.indoorTempC,
                  sample.indoor.pressurePa / 100.0f, sample.wifiRssiDbm);
}
}  // namespace telemetry