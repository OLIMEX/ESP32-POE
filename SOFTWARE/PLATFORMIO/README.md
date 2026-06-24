# PlatformIO board definitions for Olimex ESP32-POE boards

This folder contains custom PlatformIO board manifests for ESP32-POE and
ESP32-POE-ISO variants, plus a `platformio.ini` with ready-to-use environments.

## Which environment to use

- `esp32-poe`: ESP32-POE with ESP32-WROOM-32E-N4, 4 MB flash.
- `esp32-poe-16mb`: ESP32-POE with ESP32-WROOM-32E-N16, 16 MB flash.
- `esp32-poe-wrover`: ESP32-POE with ESP32-WROVER-E-N4R8, 4 MB flash and PSRAM.
- `esp32-poe-iso`: ESP32-POE-ISO with ESP32-WROOM-32E-N4, 4 MB flash.
- `esp32-poe-iso-16mb`: ESP32-POE-ISO with ESP32-WROOM-32E-N16, 16 MB flash.
- `esp32-poe-iso-wrover`: ESP32-POE-ISO with ESP32-WROVER-E-N4R8, 4 MB flash and PSRAM.

External antenna module variants use the same environment as the matching
flash/PSRAM variant.

## How to build

Run PlatformIO from this folder:

```sh
pio run -e esp32-poe
```

Replace `esp32-poe` with the environment for your board. For example:

```sh
pio run -e esp32-poe-iso-16mb
pio run -e esp32-poe-wrover
```

The `platformio.ini` sets `boards_dir = .`, so PlatformIO will find the custom
JSON files in this same folder.

## Board manifest notes

- WROOM N4 manifests set 4 MB flash.
- WROOM N16 manifests set 16 MB flash, `maximum_size: 16777216`, and Arduino
  partition table `default_16MB.csv`.
- WROVER manifests enable PSRAM with `BOARD_HAS_PSRAM` and `psram_type: qspi`.
- All manifests predefine LAN8720 Ethernet macros for Arduino `ETH.begin()`.
- WROOM variants use Ethernet clock output on GPIO17.
- WROVER variants use Ethernet clock output on GPIO0.
- Ethernet MDC is GPIO23, MDIO is GPIO18, PHY address is 0, and PHY power is
  GPIO12.

## Upload and monitor

The manifests allow USB serial upload with `esptool` and OTA upload with
`espota`. Serial monitor speed is set to 115200 in `platformio.ini`.
