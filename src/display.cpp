#include "display.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pins.h"

namespace display {

namespace {
constexpr int kScreenWidth = 128;
constexpr int kScreenHeight = 64;

Adafruit_SSD1306 oled(kScreenWidth, kScreenHeight, &SPI,
                      IoTPins::OLED_DC, IoTPins::OLED_RST, IoTPins::OLED_CS);
}  // namespace

bool begin() {
    if (!oled.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("OLED init failed. Check SPI wiring and pin map.");
        return false;
    }
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.println("IoT Kit");
    oled.display();
    return true;
}

void write(const String& line1,
           const String& line2,
           const String& line3,
           const String& line4) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println(line1);
    if (line2.length()) oled.println(line2);
    if (line3.length()) oled.println(line3);
    if (line4.length()) oled.println(line4);
    oled.display();
}

}  // namespace display
