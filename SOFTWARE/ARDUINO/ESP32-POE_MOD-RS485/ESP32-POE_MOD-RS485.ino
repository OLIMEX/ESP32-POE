/*
 * A simple example for ESP32-POE(-ISO) rev.L and MOD-RS485
 * Tested with: 
 * IDE Arduino 1.8.9
 * ESP32 package: 2.0.13
 * 
 * Setup:
 * You need 2 pair of boards ESP32-POE(-ISO) and MOD-RS485.
 * ESP32-POE(-ISO) is connected to the MOD-RS485 via UEXT cable
 * and the 2 modules with jumper wires connected the corresponding signals:
 * 3.3V to 3.3V; GND to GND; A to A; B to B.
 * The SMD jumpers on both MOD-RS485 should be closed as follow:
 * #SS/SDA to #SS
 * SCL/SCK to SCK
 * ENABLE - closed
 * 
 * One of the ESP32-POE(-ISO) should be programmed in SEND mode
 * and the second one in RECEIVE mode (check the macro MODE definition below)
 * 
 * If everything is done correctly when you open 2 terminals (of the 2 host boards)
 * when you input a character on the "Send" terminal you should receive the same
 * character on the "Receive" terminal.
 * 
 * Stanimir Petev, Olimex
 * 2023/09/27
 */

#define DE  14
#define _RE  5

#define SEND    1
#define RECEIVE 0

//#define MODE  SEND
#define MODE  RECEIVE

#define USB_Serial  Serial
#define UEXT_Serial Serial1

void Enable (int Mode)
{
  digitalWrite (DE, Mode);
  digitalWrite (_RE, Mode);
}

void setup()
{
  USB_Serial.begin (115200);
  UEXT_Serial.begin (115200);

  pinMode (DE, OUTPUT);
  pinMode (_RE, OUTPUT);

  Enable (MODE);
}

void loop ()
{
#if (MODE==SEND)
  while (USB_Serial.available() > 0)
    UEXT_Serial.print((char)USB_Serial.read());
#elif (MODE==RECEIVE)
  while (UEXT_Serial.available() > 0)
    USB_Serial.print((char)UEXT_Serial.read());
#endif
}
