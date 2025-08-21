#include <SD.h>

static const int chipSelect = 11;  // CS pin
File dataFile;

bool initSD() {
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD init failed!"));
    return false;
  }
  Serial.println(F("SD init success."));
  return true;
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
