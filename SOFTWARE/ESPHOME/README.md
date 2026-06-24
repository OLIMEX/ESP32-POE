# ESPHome configurations for Olimex ESP32-POE boards

These YAML files are starter configurations for ESP32-POE and ESP32-POE-ISO
variants.

## Which file to use

- `esp32-poe.yaml`: ESP32-POE with ESP32-WROOM-32E-N4, 4 MB flash.
- `esp32-poe-16mb.yaml`: ESP32-POE with ESP32-WROOM-32E-N16, 16 MB flash.
- `esp32-poe-wrover.yaml`: ESP32-POE with ESP32-WROVER-E-N4R8, 4 MB flash and PSRAM.
- `esp32-poe-iso.yaml`: ESP32-POE-ISO with ESP32-WROOM-32E-N4, 4 MB flash.
- `esp32-poe-iso-16mb.yaml`: ESP32-POE-ISO with ESP32-WROOM-32E-N16, 16 MB flash.
- `esp32-poe-iso-wrover.yaml`: ESP32-POE-ISO with ESP32-WROVER-E-N4R8, 4 MB flash and PSRAM.

External antenna module variants use the same YAML as the matching flash/PSRAM
variant.

## Wi-Fi or Ethernet

The files enable Wi-Fi by default. Add your credentials to `secrets.yaml`:

```yaml
wifi_ssid: "your_wifi_name"
wifi_password: "your_wifi_password"
```

To use Ethernet instead, comment the whole `wifi:` section and
`captive_portal:`, then uncomment the `ethernet:` section in the YAML.

## Notes

- WROOM variants use Ethernet clock output on GPIO17.
- WROVER variants use Ethernet clock output on GPIO0.
- WROVER variants use GPIO33 for UEXT I2C SCL because GPIO16/GPIO17 are used
  by PSRAM.
- UEXT SPI uses GPIO14 SCK, GPIO2 MOSI, GPIO15 MISO, and GPIO5 chip-select.
  These pins are shared with the microSD interface.
- The 16 MB YAMLs set `esp32.flash_size: 16MB`; ESPHome defaults to 4 MB if
  this is not specified.
