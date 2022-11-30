//demonstration for power sense and battery sense
//for Olimex ESP32-POE and ESP32-POE-ISO
//for other boards you might need to change pins in define
//some other boards might require hardware adjustments to
//enable power sense or/and battery sense

#define POWER_SENSE 39
#define BATTERY 35

void setup()
{
  Serial.begin (115200);
  pinMode (POWER_SENSE, INPUT);
  pinMode (BATTERY, INPUT);
}

void loop()
{
  Serial.print ("External power sense: ");
  Serial.println (digitalRead (POWER_SENSE));

  Serial.print ("Battery measurement: ");
  Serial.print (analogReadMilliVolts (BATTERY)*2);
  Serial.println (" mV");

  Serial.println ();

  delay (250);
}
