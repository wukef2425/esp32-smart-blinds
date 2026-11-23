#pragma once
#include <WiFiClientSecure.h>
#include <time.h>

extern const uint8_t rootcaBundle[] asm("_binary_data_cert_x509_crt_bundle_bin_start");

namespace network_security {
inline bool configure(WiFiClientSecure& client) {
    // Certificate dates cannot be verified until SNTP has set the clock.
    if (time(nullptr) < 1767225600) return false;  // 2026-01-01 UTC
    client.setCACertBundle(rootcaBundle);
    client.setHandshakeTimeout(8);
    return true;
}
}
