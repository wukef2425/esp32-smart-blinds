# Public CA bundle

`x509_crt_bundle.bin` contains public CA subject names and public keys in the Arduino-ESP32 certificate-bundle format. It contains no private key or device credentials.

- Input: https://curl.se/ca/cacert.pem (Mozilla CA extraction, header dated 2026-11-05; downloaded 2026-11-20).
- Source description and licensing: https://curl.se/docs/caextract.html
- Generator: Arduino-ESP32 `tools/gen_crt_bundle.py`, distributed with the framework source used for this project's pinned Arduino-ESP32 2.0.17 toolchain; https://github.com/espressif/arduino-esp32/blob/2.0.17/tools/gen_crt_bundle.py
- 118 root certificates; generated using `gen_crt_bundle.py --input cacert.pem`, then renamed to `x509_crt_bundle.bin`.
- Input PEM SHA-256: `427c064216761a1e72a8b30f0d954d08cb6e4435a82091fb865afd20c8497285`.
- Bundle SHA-256: `8ca330673e7ecd07610eecfaffd23aba8b9ceca10a381d3878f268348edf5aec`.

PlatformIO embeds this bundle into flash. HTTPS requires SNTP time and certificate validation; there is no insecure fallback. Refresh the bundle before a later release when trust roots change, then repeat the HTTPS preflight on the board. Keep the tested bundle fixed during the 72-hour run.
