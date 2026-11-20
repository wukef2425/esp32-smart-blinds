#include "event_log.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <stdarg.h>
#include <time.h>

namespace event_log {
namespace {
bool mounted = false;
constexpr size_t kMaxFileBytes = 512 * 1024;

void dump(const char* name) {
    if (!LittleFS.exists(name)) return;
    File file = LittleFS.open(name, FILE_READ);
    if (!file) return;
    uint8_t buffer[256];
    while (file.available()) {
        const size_t count = file.read(buffer, sizeof(buffer));
        Serial.write(buffer, count);
        delay(1);
    }
    file.close();
}
}

void begin() {
    mounted = LittleFS.begin(false);
    Serial.println(mounted ? "Log: flash ready; D = dump logs" :
                   "Log: unavailable; send F once to format log partition, then restart");
}

void write(const char* format, ...) {
    char message[384];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    const time_t utc = time(nullptr);
    char line[448];
    snprintf(line, sizeof(line), "utc=%lld ms=%lu %s", (long long)utc, millis(), message);
    Serial.print(line);
    const size_t length = strlen(line);
    if (length == 0 || line[length - 1] != '\n') Serial.println();
    if (!mounted) return;
    File file = LittleFS.open("/run.log", FILE_APPEND);
    if (!file) { Serial.println("Log: open failed"); return; }
    if (file.size() + length + 1 > kMaxFileBytes) {
        file.close();
        if (LittleFS.exists("/previous.log") && !LittleFS.remove("/previous.log")) {
            Serial.println("Log: rotation failed"); return;
        }
        if (!LittleFS.rename("/run.log", "/previous.log")) {
            Serial.println("Log: rotation failed"); return;
        }
        file = LittleFS.open("/run.log", FILE_APPEND);
    }
    const size_t written = file.print(line);
    if (length == 0 || line[length - 1] != '\n') file.println();
    file.close();
    if (written != length) Serial.println("Log: write failed");
}

void poll() {
    while (Serial.available()) {
        const char command = Serial.read();
        if (command == 'D' || command == 'd') {
            if (!mounted) { Serial.println("Log: unavailable"); continue; }
            Serial.println("LOG BEGIN");
            dump("/previous.log");
            dump("/run.log");
            Serial.println("LOG END");
        } else if (command == 'F' && !mounted) {
            if (LittleFS.format()) {
                mounted = LittleFS.begin(false);
                Serial.println(mounted ? "Log: initialized; restart before test" : "Log: mount failed");
            } else Serial.println("Log: format failed");
        }
    }
}
}
