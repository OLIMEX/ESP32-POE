This project is based on the ethernet example located in "examples/ethernet/basic" of ESP-IDF release v4.0: https://github.com/espressif/esp-idf/tree/release/v4.0

Pin numbers (RST removed and RMII clock to 17) in menuconfig are modified to match those of ESP32-PoE.

IMPORTANT: The example won't work with older (and very likely newer) versions of the ESP-IDF due to the changed structure of the libraries in the ESP-IDF.