#pragma once
#include <Arduino.h>

// SPI SSD1306 128x64 OLED. Migrated from iot_common.h.
namespace display {
// Call once in setup(). Returns true if the OLED initialized successfully.
bool begin();

// Show up to 4 lines of text. Lines 2-4 are optional.
void write(const String& line1,
           const String& line2 = "",
           const String& line3 = "",
           const String& line4 = "");
}  // namespace display
