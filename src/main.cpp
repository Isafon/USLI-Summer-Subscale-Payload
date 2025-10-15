#include <Arduino.h>
#include <SPI.h>
#include "baro_bmp280.h"
#include "rtc_pcf8523.h"
#include "uSD.h"

#define TEST_INTERVAL 1000 // 1 second

// Helper function to format timestamp
void formatTimestamp(char* buffer, size_t bufferSize, const DateTime& dt) {
  snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d", 
           dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
}

// Handle serial commands
void handleCommand(char cmd) {
  Serial.println();
  
  switch (cmd) {
    case 'l':
    case 'L':
      // Start logging
      if (isLoggingActive()) {
        Serial.println(F("Already logging to: "));
        Serial.println(getCurrentFileName());
      } else {
        Serial.println(F("Starting data logging..."));
        if (startLogging("data.csv")) {
          Serial.println(F("✓ Data logging started to data.csv"));
        } else {
          Serial.println(F("✗ Failed to start data logging!"));
        }
      }
      break;
      
    case 's':
    case 'S':
      // Stop logging
      if (isLoggingActive()) {
        Serial.println(F("Stopping data logging..."));
        if (stopLogging()) {
          Serial.println(F("✓ Logging stopped successfully!"));
        } else {
          Serial.println(F("✗ Failed to stop logging!"));
        }
      } else {
        Serial.println(F("Not currently logging"));
      }
      break;
      
    case 'd':
    case 'D':
      // Delete current log file
      if (isLoggingActive()) {
        Serial.println(F("Stopping logging..."));
        stopLogging();
        Serial.println(F("Deleting data.csv..."));
        if (deleteFile("data.csv")) {
          Serial.println(F("✓ File deleted successfully!"));
        } else {
          Serial.println(F("✗ Failed to delete file!"));
        }
      } else {
        Serial.println(F("Deleting data.csv..."));
        if (deleteFile("data.csv")) {
          Serial.println(F("✓ File deleted successfully!"));
        } else {
          Serial.println(F("✗ Failed to delete file!"));
        }
      }
      break;
      
    case 'h':
    case 'H':
      // Show help
      Serial.println(F("Commands: L=Start logging, S=Stop logging, D=Delete file"));
      break;
      
    case '\n':
    case '\r':
      break;
      
    default:
      Serial.print(F("Unknown command: "));
      Serial.println(cmd);
      Serial.println(F("Press 'H' for help"));
      break;
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("USLI Payload: Barometer + RTC + SD"));

  // Initialize SPI bus once for all SPI devices
  SPI.begin();
  Serial.println(F("✓ SPI initialized"));

  // Initialize RTC first
  if (!initRTC()) {
    Serial.println(F("✗ RTC failed!"));
    while (1) delay(1000);
  }
  Serial.println(F("✓ RTC OK"));
  
  // Check if RTC needs time setting
  DateTime currentTime;
  if (readRTC(currentTime)) {
    Serial.print(F("Current RTC time: "));
    Serial.print(currentTime.year); Serial.print(F("-"));
    Serial.print(currentTime.month); Serial.print(F("-"));
    Serial.print(currentTime.day); Serial.print(F(" "));
    Serial.print(currentTime.hour); Serial.print(F(":"));
    Serial.print(currentTime.minute); Serial.print(F(":"));
    Serial.println(currentTime.second);
    
    // Check if time seems reasonable (not default/unset)
    bool isDefaultTime = (currentTime.year < 2024 || 
                         (currentTime.hour == 12 && currentTime.minute < 5));
    
    if (isDefaultTime) {
      Serial.println(F("RTC time appears unset. Setting to current time..."));
      if (setRTC(2025, 10, 14, 21, 8, 20)) {  // Update this to current time
        Serial.println(F("✓ RTC time set successfully!"));
      } else {
        Serial.println(F("✗ Failed to set RTC time!"));
      }
    } else {
      Serial.println(F("✓ RTC time is valid, leaving it unchanged."));
    }
  } else {
    Serial.println(F("Could not read RTC time. Setting default time..."));
    if (setRTC(2025, 10, 14, 21, 07, 0)) {  // Update this to current time
      Serial.println(F("✓ RTC time set successfully!"));
    } else {
      Serial.println(F("✗ Failed to set RTC time!"));
    }
  }

  // Initialize barometer
  if (!initBaro()) {
    Serial.println(F("✗ Barometer failed!"));
    while (1) delay(1000);
  }
  Serial.println(F("✓ Barometer OK"));

  // Initialize microSD card
  if (!initSD()) {
    Serial.println(F("✗ SD card failed!"));
    while (1) delay(1000);
  }
  Serial.println(F("✓ SD card OK"));

  // SD card ready for manual logging control
  Serial.println(F("✓ SD card ready - use L to start logging"));

  Serial.println(F("\nTimestamp\t\t\tTemp(°C)\tPressure(hPa)\tAltitude(m)"));
  Serial.println(F("================================================================="));
  Serial.println(F("Commands: L=Start logging, S=Stop logging, D=Delete file, H=Help"));
}

void loop() {
  // Read RTC data
  DateTime dt;
  char timestamp[32];
  
  if (!readRTC(dt)) {
    Serial.println(F("✗ Failed to read RTC!"));
    delay(TEST_INTERVAL);
    return;
  }
  
  formatTimestamp(timestamp, sizeof(timestamp), dt);
  
  // Read barometer data
  BaroData data;
  if (!readBaro(data)) {
    Serial.print(F("["));
    Serial.print(timestamp);
    Serial.println(F("] ✗ Invalid barometer read!"));
    delay(TEST_INTERVAL);
    return;
  }

  // Print timestamp and sensor data
  Serial.print(F("["));
  Serial.print(timestamp);
  Serial.print(F("] "));
  Serial.print(data.temperature, 2);
  Serial.print(F("\t\t"));
  Serial.print(data.pressure, 2);
  Serial.print(F("\t\t"));
  Serial.println(data.altitude, 2);

  // Write data to SD card
  if (!writeData(dt, data)) {
    Serial.println(F("✗ Failed to write to SD card!"));
  }

  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  delay(TEST_INTERVAL);
}
