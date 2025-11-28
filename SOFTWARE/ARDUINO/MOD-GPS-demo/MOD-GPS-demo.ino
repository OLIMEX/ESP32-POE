/*
 * ESP32-POE-ISO + Olimex MOD-GPS (UEXT UART)
 * -----------------------------------------
 * Board UART pins (per ESP32-POE-ISO schematic):
 *   GPIO4  -> UEXT TXD -> GPS RX
 *   GPIO36 -> UEXT RXD -> GPS TX   (input only, OK for RX)
 *
 * GPS modules (uBlox NEO-6M / NEO-7M / NEO-M8) default baud: 9600
 *
 * This demo simply prints all NMEA sentences to USB Serial.
 */

#include <HardwareSerial.h>

// UART2 instance
HardwareSerial GPS(2);

// ESP32-POE-ISO UEXT UART pins
#define GPS_TX 4    // ESP32 TX → GPS RX
#define GPS_RX 36   // GPS TX → ESP32 RX (input only)

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(10); }

  Serial.println("ESP32-POE-ISO + MOD-GPS Demo");
  Serial.println("Reading NMEA sentences...\n");

  // Start UART2 using correct pins
  GPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
}

void loop() {
  // If GPS has data available, read and echo it
  while (GPS.available()) {
    char c = GPS.read();
    Serial.print(c);
  }
}
