// Demo iDemonstration for power sense and battery sense
// Compatible with Olimex ESP32-POE, ESP32-POE-ISO, ESP32-POE2
// Can be configured for other boards changing the #defines
// Automatically selects ADC attenuation based on resistor divider

#include "esp32-hal-adc.h" // ensures adc_attenuation_t is visible

// These are GPIO pins for ESP32-POE, ESP32-POE-ISO, ESP32-POE2 edit for other boards
#define POWER_SENSE 39
#define BATTERY     35

// Adjustable resistor divider coefficient (rC = R1+R7 / R7), edit for other boards
float resistorCoefficient = 2.0;  

// Define maximum input voltage (e.g. battery charger max voltage)
const float batteryMaxVoltage = 4.2;  // volts
// ---------------------------------------------

// Function to auto-select attenuation based on expected voltage range
adc_attenuation_t selectAttenuation(float expectedInputV)
{
  if (expectedInputV <= 1.1)
    return ADC_0db;     // 0–1.1 V
  else if (expectedInputV <= 1.5)
    return ADC_2_5db;   // 0–1.5 V
  else if (expectedInputV <= 2.2)
    return ADC_6db;     // 0–2.2 V
  else
    return ADC_11db;    // 0–3.9 V
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  pinMode(POWER_SENSE, INPUT);
  pinMode(BATTERY, INPUT);

  // Compute expected ADC input voltage based on divider
  float expectedADCVoltage = batteryMaxVoltage / resistorCoefficient;

  // Automatically choose proper attenuation
  adc_attenuation_t batteryAtten = selectAttenuation(expectedADCVoltage);
  adc_attenuation_t powerSenseAtten = ADC_0db; // stays digital but defined for completeness

  // Apply ADC settings
  analogSetPinAttenuation(BATTERY, batteryAtten);
  analogSetPinAttenuation(POWER_SENSE, powerSenseAtten);

  Serial.println("ESP32 Power & Battery Sense Demo");
  Serial.print("Expected ADC voltage: ");
  Serial.print(expectedADCVoltage, 2);
  Serial.print(" V → Using attenuation: ");
  Serial.println((batteryAtten == ADC_0db) ? "0" :
                 (batteryAtten == ADC_2_5db) ? "2_5" :
                 (batteryAtten == ADC_6db) ? "6" : "11");
}

void loop()
{
  // External power sense (digital)
  Serial.print("External power sense: ");
  Serial.println(digitalRead(POWER_SENSE));

  // Battery measurement (analog)
  int battery_mV = analogReadMilliVolts(BATTERY);
  float adjusted_mV = battery_mV * resistorCoefficient;

  Serial.print("Battery measurement: ");
  Serial.print(adjusted_mV, 1);
  Serial.println(" mV");

  Serial.println();
  delay(500);
}
