# ESP32 Smart Blinds

A temperature-based smart blinds project built with ESP32, PlatformIO, and Arduino.

## Hardware

| Component | Connection / configuration |
| --- | --- |
| ESP32 DevKit | USB 5V power; PlatformIO board: `esp32dev` |
| BMP280 (I2C) | SDA → GPIO21, SCL → GPIO22, GND → GND; 3.3V power |
| SG90 servo | Signal → GPIO13; 5V supply; common ground with ESP32 |
| ThingSpeak | Planned upload interval: 5 minutes (`300000` ms) |
| Open-Meteo | Outdoor weather data (planned) |

## Project Layout

- `src/main.cpp`: initialization and non-blocking scheduling.
- `sensors`: indoor temperature and pressure from BMP280.
- `weather`: Open-Meteo weather data and freshness checks.
- `blinds`: SG90 angle control.
- `wifi_manager`: Wi-Fi connection and reconnection.
- `telemetry`: ThingSpeak field mapping and uploads.
- `controller`: temperature rules, hysteresis, and fault handling.
- `include/config.h`: pins, intervals, and non-sensitive settings.

Module headers are in `include/`; implementations are in `src/`.

## Build and Upload

Requires PlatformIO with the Arduino framework.

Dependencies are pinned in `platformio.ini`: Espressif32 6.9.0, Arduino-ESP32 2.0.17 (package `3.20017.241212+sha.dcc1105b`), BMP280 2.6.8, Unified Sensor 1.1.15, BusIO 1.17.4, ESP32Servo 1.1.2, and ArduinoJson 7.4.2.

```sh
pio run
pio run -t upload
pio device monitor
```

`include/secrets.example.h` defines the planned Wi-Fi and ThingSpeak credentials.

Serial speed: `115200`.
