#include <SD.h>
#include "uSD.h"

#define SD_CS_PIN 9  // CS pin from config.h
static const int chipSelect = SD_CS_PIN;
File dataFile;

bool initSD() {
  return SD.begin(chipSelect);
}

bool logData(const String &data) {
  dataFile = SD.open("flight.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println(F("Error opening flight.txt"));
    return false;
  }

  dataFile.println(data);   // Write a line of telemetry
  dataFile.close();
  return true;
}
