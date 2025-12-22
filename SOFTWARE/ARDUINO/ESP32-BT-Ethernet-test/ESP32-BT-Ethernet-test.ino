/*
====================================================================
OLIMEX ESP32-POE or Olimex ESP32-POE-ISO
Ethernet + Bluetooth Classic (SPP) Demo
====================================================================

HOW TO USE THIS DEMO
-------------------

1. Arduino IDE SETTINGS
   --------------------
   - Board:              OLIMEX ESP32-POE or OLIMEX ESP32-POE-ISO
   - Flash Size:         4MB
   - Partition Scheme:   Huge APP (3MB No OTA)   [REQUIRED]
   - CPU Frequency:      240 MHz
   - PSRAM:              Disabled

2. FLASHING
   --------
   Compile and upload this sketch to the board.
   Open the USB Serial Monitor at 115200 baud.

3. ETHERNET
   --------
   - Connect an Ethernet cable to the board
   - After boot you should see messages such as:
       Ethernet started
       Ethernet connected
       Ethernet IP: x.x.x.x

   - Ethernet status is printed:
       * to USB Serial
       * to Bluetooth (every 2 seconds)

4. BLUETOOTH (ANDROID)
   -------------------
   - Install the app:
       "Serial Bluetooth Terminal"
       Author: Kai Morich
       (from Google Play Store)

   - Enable Bluetooth on the phone
   - Pair with the device named:
       ESP32-POE-ISO-BT

   - Open the app and connect to the device

5. BLUETOOTH USAGE
   ---------------
   - Any text typed in the Bluetooth terminal is echoed back
   - The same text appears on the USB Serial Monitor
   - Every 2 seconds the board sends Ethernet status over Bluetooth:
       ETH OK, IP=192.168.x.x
       or
       ETH DOWN

6. NOTES
   -----
   - Bluetooth Classic uses significant flash memory
   - The "Huge APP" partition scheme is mandatory
   - WiFi is not used; only Ethernet + Bluetooth run simultaneously
   - Tested with ESP32 Arduino core 3.3.x

====================================================================
*/


#include <ETH.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

/* ESP32-POE-ISO Ethernet configuration */
#define ETH_PHY_TYPE    ETH_PHY_LAN8720
#define ETH_PHY_ADDR    0
#define ETH_PHY_MDC     23
#define ETH_PHY_MDIO    18
#define ETH_PHY_POWER   12
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT

static bool eth_connected = false;

/* Event handler (ESP32 core 3.x) */
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {

    case ARDUINO_EVENT_ETH_START:
      Serial.println("Ethernet started");
      ETH.setHostname("esp32-poe-iso");
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("Ethernet connected");
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("Ethernet IP: ");
      Serial.println(ETH.localIP());
      eth_connected = true;
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet disconnected");
      eth_connected = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("Ethernet stopped");
      eth_connected = false;
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP32-POE-ISO Ethernet + Bluetooth demo");

  /* Power LAN8720 PHY */
  pinMode(ETH_PHY_POWER, OUTPUT);
  digitalWrite(ETH_PHY_POWER, HIGH);
  delay(10);

  /* Register Ethernet events */
  WiFi.onEvent(WiFiEvent);

  /* Start Ethernet */
  ETH.begin(
    ETH_PHY_TYPE,
    ETH_PHY_ADDR,
    ETH_PHY_MDC,
    ETH_PHY_MDIO,
    ETH_PHY_POWER,
    ETH_CLK_MODE
  );

  /* Start Bluetooth Classic */
  if (!SerialBT.begin("ESP32-POE-ISO-BT")) {
    Serial.println("Bluetooth start failed");
  } else {
    Serial.println("Bluetooth started: ESP32-POE-ISO-BT");
  }
}

void loop() {
  /* Echo Bluetooth input */
  if (SerialBT.available()) {
    char c = SerialBT.read();
    SerialBT.write(c);
    Serial.write(c);
  }

  /* Periodic Ethernet status (every 2 seconds) */
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus >= 2000) {
    lastStatus = millis();

    if (eth_connected) {
      String msg = "ETH OK, IP=" + ETH.localIP().toString();
      Serial.println(msg);
      SerialBT.println(msg);
    } else {
      Serial.println("ETH DOWN");
      SerialBT.println("ETH DOWN");
    }
  }
}
