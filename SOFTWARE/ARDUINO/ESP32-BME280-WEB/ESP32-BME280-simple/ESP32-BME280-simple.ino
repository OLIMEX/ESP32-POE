/*
  =====================================================================
  PROJECT: ESP32 Environmental Web Station (ESPUI + BME280)
  =====================================================================

  This project turns an ESP32 board into a small WiFi environmental
  monitoring station with a simple live web interface.

  The device measures:
      • Temperature
      • Humidity
      • Air Pressure

  The web page (served directly by the ESP32) shows:
      • Current sensor readings
      • Real clock time from the Internet (NTP)
      • Minimum recorded values + time of occurrence
      • Maximum recorded values + time of occurrence

  No external server or cloud is required.
  Everything runs directly on the ESP32.

  =====================================================================
  SUPPORTED ESP32 BOARDS (AUTO-DETECTED)

  - Olimex ESP32-POE
  - Olimex ESP32-POE-ISO
  - Olimex ESP32-POE2
  - Olimex ESP32-EVB
  - Olimex ESP32-C3-DevKit-Lipo

  The sketch automatically selects correct I2C pins depending on
  the board you choose in Arduino IDE.

  ⚠ If you select the wrong board → wrong pins → sensor will not work.

  =====================================================================
  SENSOR BOARD

  - Olimex MOD-BME280 (Temperature / Humidity / Pressure)

  =====================================================================
  ARDUINO IDE SETUP

  1. Install Arduino IDE
  2. Install ESP32 core:
     https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html

  3. Select board in:
     Tools → Board

        ESP32-POE / POE2 / POE-ISO → "Olimex ESP32-POE"
        ESP32-EVB                  → "Olimex ESP32-EVB"
        ESP32-C3-DevKit-Lipo       → "Olimex ESP32-C3-DevKit-Lipo"

  4. For ESP32-C3-DevKit-Lipo:
     Enable "USB CDC on boot" in Tools menu, otherwise Serial Monitor
     will not work (this board uses native USB).

  =====================================================================
  REQUIRED ARDUINO LIBRARIES

  Install via:
  Sketch → Include Library → Manage Libraries

  • ESPUI
  • Adafruit BME280
  • Adafruit Unified Sensor

  Board support package:
  Boards Manager → "esp32 by Espressif Systems"

  =====================================================================
  HARDWARE CONNECTION (I2C)

  Pins are chosen automatically in software.

  ➜ ESP32-POE / POE2 / POE-ISO / EVB
      Plug MOD-BME280 directly into UEXT connector.

  ➜ ESP32-C3-DevKit-Lipo (NO UEXT connector!)
      Use wires:

         BME280  →  ESP32-C3-DevKit-Lipo
         3.3V    →  3V3
         GND     →  GND
         SDA     →  GPIO 8
         SCL     →  GPIO 9

  =====================================================================
  WHAT HAPPENS WHEN YOU RUN THIS

  1. ESP32 connects to your WiFi network
  2. ESP32 gets real date/time from Internet (NTP server)
  3. ESP32 initializes the BME280 sensor over I2C
  4. ESP32 starts a web server using ESPUI library
  5. Browser interface shows live environmental data
  6. Min/Max values are tracked continuously

  =====================================================================
  HOW TO USE

  1. Enter your WiFi name and password in the sketch.
  2. Upload code to the board.
  3. Open Tools → Serial Monitor (115200 baud).
  4. After connection, an IP address will be shown.
  5. Type that IP address into your web browser.

  Example:
      http://192.168.1.55

  =====================================================================
  NOTES FOR BEGINNERS

  • Time is taken from the Internet — no RTC module needed.
  • Min/Max values reset every time the board restarts.
  • If sensor is not detected, check wiring and board selection.
  • The ESP32 both measures data AND hosts the webpage.

  =====================================================================
*/


#include <WiFi.h>
#include <ESPUI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <time.h>   // Required for NTP time

/* ---------------- WIFI SETTINGS ---------------- */
const char* ssid     = "WIFI-NAME";
const char* password = "WIFI-PASSWORD";

/* ---------------- TIME SETTINGS ---------------- */
// Bulgaria example: UTC+2 winter, UTC+3 summer
const long gmtOffset_sec = 2 * 3600;
const int  daylightOffset_sec = 3600;

/* =========================================================
   BOARD AUTO-DETECTION (I2C PINS)
   ========================================================= */
#if defined(ARDUINO_ESP32_POE) || defined(ARDUINO_ESP32_POE_ISO) || defined(ARDUINO_ESP32_POE2)
  #define SDA_PIN 13
  #define SCL_PIN 16
#elif defined(ARDUINO_ESP32_EVB)
  #define SDA_PIN 13
  #define SCL_PIN 16
#elif defined(ARDUINO_ESP32C3_DEVKIT_LIPO)
  #define SDA_PIN 8
  #define SCL_PIN 9
#else
  #define SDA_PIN 21
  #define SCL_PIN 22
#endif

Adafruit_BME280 bme;

/* ---------------- ESPUI IDs ---------------- */
uint16_t tempNow, humNow, presNow;
uint16_t tempMinBox, tempMaxBox;
uint16_t humMinBox, humMaxBox;
uint16_t presMinBox, presMaxBox;
uint16_t clockLabel;

/* ---------------- MIN/MAX STORAGE ---------------- */
float tMin = 1000, tMax = -1000;
float hMin = 1000, hMax = -1000;
float pMin = 10000, pMax = -10000;

String tMinTime, tMaxTime;
String hMinTime, hMaxTime;
String pMinTime, pMaxTime;

/* ---------------- GET CURRENT TIME STRING ---------------- */
String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Time err";

  char buf[20];
  strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
  return String(buf);
}

/* ========================================================= */

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  /* -------- WIFI -------- */
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  /* -------- GET INTERNET TIME -------- */
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  Serial.println("Waiting for time sync...");
  delay(2000);

  /* -------- BME280 -------- */
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    Serial.println("BME280 not detected!");
    while (1);
  }

  /* -------- START WEB UI -------- */
  ESPUI.begin("Environmental Station");

  /* CURRENT VALUES (Distinct colors) */
  tempNow = ESPUI.addControl(Label, "Temperature °C", "--", ControlColor::Carrot);
  humNow  = ESPUI.addControl(Label, "Humidity %", "--", ControlColor::Emerald);
  presNow = ESPUI.addControl(Label, "Pressure hPa", "--", ControlColor::Peterriver);

  /* CLOCK */
  clockLabel = ESPUI.addControl(Label, "Current Time", "--:--:--", ControlColor::None);

  /* MIN/MAX */
  tempMinBox = ESPUI.addControl(Label, "Min Temp", "-", ControlColor::Carrot);
  tempMaxBox = ESPUI.addControl(Label, "Max Temp", "-", ControlColor::Carrot);

  humMinBox  = ESPUI.addControl(Label, "Min Humidity", "-", ControlColor::Emerald);
  humMaxBox  = ESPUI.addControl(Label, "Max Humidity", "-", ControlColor::Emerald);

  presMinBox = ESPUI.addControl(Label, "Min Pressure", "-", ControlColor::Peterriver);
  presMaxBox = ESPUI.addControl(Label, "Max Pressure", "-", ControlColor::Peterriver);
}

/* ========================================================= */

void loop() {

  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0F;
  String nowTime = getTimeString();

  /* ---- UPDATE CURRENT VALUES ---- */
  ESPUI.updateLabel(tempNow, String(t, 1));
  ESPUI.updateLabel(humNow,  String(h, 1));
  ESPUI.updateLabel(presNow, String(p, 1));
  ESPUI.updateLabel(clockLabel, nowTime);

  /* ---- MIN/MAX LOGIC ---- */
  if (t < tMin) { tMin = t; tMinTime = nowTime; }
  if (t > tMax) { tMax = t; tMaxTime = nowTime; }

  if (h < hMin) { hMin = h; hMinTime = nowTime; }
  if (h > hMax) { hMax = h; hMaxTime = nowTime; }

  if (p < pMin) { pMin = p; pMinTime = nowTime; }
  if (p > pMax) { pMax = p; pMaxTime = nowTime; }

  /* ---- DISPLAY MIN/MAX ---- */
  ESPUI.updateLabel(tempMinBox, "Min: " + String(tMin,1) + " @ " + tMinTime);
  ESPUI.updateLabel(tempMaxBox, "Max: " + String(tMax,1) + " @ " + tMaxTime);

  ESPUI.updateLabel(humMinBox,  "Min: " + String(hMin,1) + " @ " + hMinTime);
  ESPUI.updateLabel(humMaxBox,  "Max: " + String(hMax,1) + " @ " + hMaxTime);

  ESPUI.updateLabel(presMinBox, "Min: " + String(pMin,1) + " @ " + pMinTime);
  ESPUI.updateLabel(presMaxBox, "Max: " + String(pMax,1) + " @ " + pMaxTime);

  delay(2000);
}
