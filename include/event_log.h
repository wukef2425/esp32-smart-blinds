#pragma once

namespace event_log {
void begin();
void write(const char* format, ...);
void poll();
}
