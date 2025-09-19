#include <Arduino.h>

void printChipInfo() {
  // Timestamp in seconds since boot
  Serial.printf("\n[Uptime: %lu seconds]\n", millis() / 1000);

  Serial.println("=== ESP32 Chip Information ===");

  // Basic chip info
  Serial.printf("Chip model: %s\n", ESP.getChipModel());
  Serial.printf("Chip revision: %d\n", ESP.getChipRevision());
  Serial.printf("Chip cores: %d\n", ESP.getChipCores());

  // Flash info
  Serial.printf("Flash size: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("Flash speed: %u Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash mode: %u\n", ESP.getFlashChipMode());

  // Heap info
  Serial.printf("Heap size: %u bytes\n", ESP.getHeapSize());
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Min free heap: %u bytes\n", ESP.getMinFreeHeap());
  Serial.printf("Max alloc heap: %u bytes\n", ESP.getMaxAllocHeap());

  // PSRAM (if present)
  if (ESP.getPsramSize() > 0) {
    Serial.printf("PSRAM size: %u bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
  } else {
    Serial.println("No PSRAM detected");
  }

  // Unique chip ID (from eFuse MAC)
  uint64_t chipid = ESP.getEfuseMac();
  Serial.printf("Chip ID (MAC): %04X%08X\n",
                (uint16_t)(chipid >> 32), (uint32_t)chipid);

  Serial.println("==============================\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  printChipInfo(); // first print on boot
}

void loop() {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();

  if (now - lastPrint >= 30000) { // every 30 seconds
    printChipInfo();
    lastPrint = now;
  }
}
