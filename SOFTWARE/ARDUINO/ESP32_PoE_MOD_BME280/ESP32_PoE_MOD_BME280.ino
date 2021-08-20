/*
  Simple demo for Olimex boards: ESP32-PoE and MOD-BME280.
  Reads data from the module and prints it on the terminal.
  This sketch requires Espressif package -
  how to install it read here: https://github.com/espressif/arduino-esp32
*/

#include <Wire.h>
#include "Adafruit_BME280.h" // Main menu -> Sketch -> Include Library -> Manage Libraries -> search for "Adafruit BME280 Library" and install it

#define SEALEVELPRESSURE_HPA (1013.25)  // required for MOD-BME280

Adafruit_BME280 bme; // I2C

void setup()
{
  Serial.begin(115200); // initialize serial communications
  Wire.begin (13, 16);  // init I2C on the respective pins
  bme.begin(0x77);
}

char Buff[100];
float Temperature, Pressure, Altitude, Humidity;
void loop ()
{
  Temperature = bme.readTemperature();
  Pressure = bme.readPressure() / 100.0F;
  Altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Humidity = bme.readHumidity();
  sprintf (Buff, "Temperature = %.2f C\nPressure = %.2f hPa\nAltitude = %.2f m\nHumidity = %.2f %%\n\n\n", Temperature, Pressure, Altitude, Humidity);
  Serial.print (Buff);
  delay (1000);
}
