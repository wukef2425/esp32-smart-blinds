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

bool wifiOk = false;
bool tempOk = false;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    delay(600);
    Serial.println();
    Serial.println("========================================");
    Serial.println("IoT@UCI Team 22 Final Project: ESP32 Smart Blinds");
    Serial.println("========================================");

    // Initialize I2C bus
    Wire.begin(IoTPins::I2C_SDA, IoTPins::I2C_SCL);

    /// Initialize dashboard display
    display::begin();
    display::write("IoT@UCI Smart Blinds", "Initializing...");

    // Connect to WiFi
    display::write("Joining WiFi...", WIFI_SSID);
    wifiOk = wifi_manager::begin();

    if (wifiOk) {
        display::write("WiFi Connected", WiFi.localIP().toString());
    } else {
        display::write("WiFi Connection Failed", "Check secrets.h");
    }

    // Setup sensors
    display::write("Setting up sensors...", "BMP280 Temperature Sensor");
    tempOk = sensors::tempBegin();
    display::write("BMP280 Status", tempOk ? "OK at 0x76" : "Not found");
}

void loop() {
    // Read temperature and pressure
    sensors::Reading tempReading;
    if (tempOk) {
        sensors::tempRead(tempReading);
    }

    // TODO: Update weather information
    // weather::update();

    // Display current status on the OLED if valid
    if (tempReading.valid) {
        display::write(
            "Temp: " + String(tempReading.indoorTempC, 1) + " C",
            "Pressure: " + String(tempReading.pressurePa / 100.0f, 1) + " hPa"
        );
    } else {
        display::write("Temp Sensor Error", "Check wiring");
    }
}
