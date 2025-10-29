#include <SD.h>
#include "config.h"
#include "uSD.h"

// Global file handle
File dataFile;
bool isLogging = false;
char currentFileName[32] = "";

bool initSD() {
  Serial.print(F("SD: Initializing with CS pin "));
  Serial.println(SD_CS_PIN);
  
  if (SD.begin(SD_CS_PIN)) {
    Serial.println(F("SD: Initialization successful"));
    return true;
  } else {
    Serial.println(F("SD: Initialization failed"));
    return false;
  }
}

bool startLogging(const char* fileName) {
  if (isLogging) {
    Serial.println(F("SD: Already logging"));
    return false; // Already logging
  }
  
  Serial.print(F("SD: Opening file "));
  Serial.println(fileName);
  
  // Open file for appending (FILE_WRITE appends to existing files)
  dataFile = SD.open(fileName, FILE_WRITE);
  if (!dataFile) {
    Serial.println(F("SD: Failed to open file"));
    Serial.println(F("SD: Check wiring and card"));
    return false; // Failed to open file
  }
  
  Serial.println(F("SD: File opened successfully"));
  
  // Write CSV header only if file is new (size = 0)
  if (dataFile.size() == 0) {
    dataFile.println(F("Timestamp,Temp_C,Pressure_hPa,Altitude_m"));
    Serial.println(F("SD: New file - header added"));
  } else {
    Serial.println(F("SD: Appending to existing file"));
  }
  
  isLogging = true;
  strncpy(currentFileName, fileName, sizeof(currentFileName) - 1);
  currentFileName[sizeof(currentFileName) - 1] = '\0';
  return true;
}

bool stopLogging() {
  if (!isLogging) {
    return false; // Not logging
  }
  
  dataFile.close();
  isLogging = false;
  currentFileName[0] = '\0';
  return true;
}

bool writeData(const DateTime& dt, const BaroData& data) {
  if (!isLogging) {
    Serial.println(F("SD: Not logging"));
    return false;
  }
  
  if (!dataFile) {
    Serial.println(F("SD: File not open"));
    return false;
  }
  
  // Format: YYYY-MM-DD HH:MM:SS,Temp,Pressure,Altitude
  dataFile.print(dt.year);
  dataFile.print(F("-"));
  if (dt.month < 10) dataFile.print(F("0"));
  dataFile.print(dt.month);
  dataFile.print(F("-"));
  if (dt.day < 10) dataFile.print(F("0"));
  dataFile.print(dt.day);
  dataFile.print(F(" "));
  if (dt.hour < 10) dataFile.print(F("0"));
  dataFile.print(dt.hour);
  dataFile.print(F(":"));
  if (dt.minute < 10) dataFile.print(F("0"));
  dataFile.print(dt.minute);
  dataFile.print(F(":"));
  if (dt.second < 10) dataFile.print(F("0"));
  dataFile.print(dt.second);
  dataFile.print(F(","));
  dataFile.print(data.temperature, 2);
  dataFile.print(F(","));
  dataFile.print(data.pressure, 2);
  dataFile.print(F(","));
  dataFile.println(data.altitude, 2);
  
  dataFile.flush(); // Force write to card
  
  // Check if write was successful
  if (dataFile.getWriteError()) {
    Serial.println(F("SD: Write error detected"));
    return false;
  }
  
  return true;
}

// Write event data to SD card
bool writeData(const DateTime& dt, const char* event, const char* message) {
  if (!isLogging) {
    return false;
  }
  
  if (!dataFile) {
    return false;
  }
  
  // Format: YYYY-MM-DD HH:MM:SS,EVENT,Event_Message,,
  dataFile.print(dt.year);
  dataFile.print(F("-"));
  if (dt.month < 10) dataFile.print(F("0"));
  dataFile.print(dt.month);
  dataFile.print(F("-"));
  if (dt.day < 10) dataFile.print(F("0"));
  dataFile.print(dt.day);
  dataFile.print(F(" "));
  if (dt.hour < 10) dataFile.print(F("0"));
  dataFile.print(dt.hour);
  dataFile.print(F(":"));
  if (dt.minute < 10) dataFile.print(F("0"));
  dataFile.print(dt.minute);
  dataFile.print(F(":"));
  if (dt.second < 10) dataFile.print(F("0"));
  dataFile.print(dt.second);
  dataFile.print(F(","));
  dataFile.print(event);
  dataFile.print(F(","));
  dataFile.print(message);
  dataFile.print(F(",,"));
  dataFile.println();
  
  dataFile.flush();
  return true;
}

bool deleteFile(const char* fileName) {
  if (isLogging && strcmp(currentFileName, fileName) == 0) {
    return false; // Can't delete currently open file
  }
  
  return SD.remove(fileName);
}


bool isLoggingActive() {
  return isLogging;
}

const char* getCurrentFileName() {
  return currentFileName;
}
