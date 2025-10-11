/*
 * SD Card Endurance Test - 16GB Micro SD
 *
 * This sketch continuously writes telemetry data to an SD card
 * to measure how long it can log data before filling up.
 *
 * Simulates rocket telemetry data format at different logging rates:
 * - 1 Hz (preflight/postflight)
 * - 10 Hz (normal flight)
 * - 20 Hz (high-speed logging)
 *
 * Estimates time until card is full based on:
 * - Data rate (bytes/second)
 * - Available space on card
 * - Current write speed
 *
 * Hardware:
 * - SD Card Module connected to Pin 11 (CS)
 * - 16GB Micro SD card (FAT32 formatted)
 * - Status LED on Pin 13
 *
 * Serial Commands:
 * - '1' = Start 1 Hz logging test
 * - '2' = Start 10 Hz logging test
 * - '3' = Start 20 Hz logging test
 * - '4' = Fast fill test (max speed)
 * - 's' = Stop logging
 * - 'i' = Show card info
 * - 'd' = Delete test files
 * - 'h' = Show help
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// Pin definitions
#define SD_CS_PIN 11
#define STATUS_LED_PIN 13

// Test parameters
#define BUFFER_SIZE 512  // Write in 512-byte blocks for efficiency

// Test modes
enum TestMode {
  MODE_IDLE,
  MODE_1HZ,
  MODE_10HZ,
  MODE_20HZ,
  MODE_MAX_SPEED
};

// Global state
struct TestState {
  TestMode mode;
  bool logging;
  File dataFile;
  String fileName;

  // Statistics
  unsigned long recordsWritten;
  unsigned long bytesWritten;
  unsigned long startTime;
  unsigned long lastWrite;
  unsigned long writeErrors;

  // Timing
  unsigned long writeInterval;  // milliseconds between writes

  // Performance metrics
  unsigned long minWriteTime;
  unsigned long maxWriteTime;
  unsigned long totalWriteTime;
  unsigned long writeCount;
} test;

// Card information
struct CardInfo {
  uint32_t cardSize;       // MB
  uint32_t freeSpace;      // MB
  uint32_t usedSpace;      // MB
  float percentUsed;
} cardInfo;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  Serial.println(F("========================================"));
  Serial.println(F("SD Card Endurance Test - 16GB"));
  Serial.println(F("========================================\n"));

  // Initialize SD card
  Serial.print(F("Initializing SD card... "));
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("✗ FAILED!"));
    Serial.println(F("Check:"));
    Serial.println(F("  - Card is inserted"));
    Serial.println(F("  - Card is FAT32 formatted"));
    Serial.println(F("  - CS pin connected to Pin 11"));
    while (1) {
      digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
      delay(200);
    }
  }
  Serial.println(F("✓ OK\n"));

  // Get card information
  updateCardInfo();
  printCardInfo();

  // Initialize test state
  test.mode = MODE_IDLE;
  test.logging = false;
  test.recordsWritten = 0;
  test.bytesWritten = 0;
  test.writeErrors = 0;
  test.minWriteTime = 999999;
  test.maxWriteTime = 0;
  test.totalWriteTime = 0;
  test.writeCount = 0;

  printMenu();
  Serial.println(F("Ready for testing!\n"));
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  // Perform logging if active
  if (test.logging) {
    unsigned long now = millis();

    if (now - test.lastWrite >= test.writeInterval) {
      writeDataRecord();
      test.lastWrite = now;

      // Print status every 100 records
      if (test.recordsWritten % 100 == 0) {
        printStatus();
      }

      // Update card info every 1000 records
      if (test.recordsWritten % 1000 == 0) {
        updateCardInfo();
        printProjection();
      }

      // Check if card is getting full
      if (cardInfo.percentUsed > 95.0) {
        Serial.println(F("\n⚠️  WARNING: Card is 95% full!"));
        Serial.println(F("Stopping test to prevent card from filling completely.\n"));
        stopLogging();
      }
    }
  }

  // Blink status LED
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    digitalWrite(STATUS_LED_PIN, test.logging ? HIGH : LOW);
    if (!test.logging && millis() % 1000 < 500) {
      digitalWrite(STATUS_LED_PIN, HIGH);
    } else if (!test.logging) {
      digitalWrite(STATUS_LED_PIN, LOW);
    }
    lastBlink = millis();
  }
}

void handleCommand(char cmd) {
  Serial.println();

  switch (cmd) {
    case '1':
      startLogging(MODE_1HZ, "log_1hz.csv", 1000);
      break;

    case '2':
      startLogging(MODE_10HZ, "log_10hz.csv", 100);
      break;

    case '3':
      startLogging(MODE_20HZ, "log_20hz.csv", 50);
      break;

    case '4':
      startLogging(MODE_MAX_SPEED, "log_max.csv", 0);
      break;

    case 's':
    case 'S':
      if (test.logging) {
        stopLogging();
      } else {
        Serial.println(F("Not currently logging"));
      }
      break;

    case 'i':
    case 'I':
      updateCardInfo();
      printCardInfo();
      break;

    case 'd':
    case 'D':
      deleteTestFiles();
      break;

    case 'h':
    case 'H':
      printMenu();
      break;

    case '\n':
    case '\r':
      break;

    default:
      Serial.print(F("Unknown command: "));
      Serial.println(cmd);
      break;
  }
}

void startLogging(TestMode mode, const char* fileName, unsigned long interval) {
  if (test.logging) {
    Serial.println(F("Already logging! Stop first."));
    return;
  }

  test.mode = mode;
  test.fileName = fileName;
  test.writeInterval = interval;
  test.recordsWritten = 0;
  test.bytesWritten = 0;
  test.writeErrors = 0;
  test.minWriteTime = 999999;
  test.maxWriteTime = 0;
  test.totalWriteTime = 0;
  test.writeCount = 0;
  test.startTime = millis();
  test.lastWrite = millis();

  Serial.print(F("Starting logging: "));
  Serial.println(getModeString(mode));
  Serial.print(F("File: "));
  Serial.println(test.fileName);
  Serial.print(F("Rate: "));
  if (mode == MODE_MAX_SPEED) {
    Serial.println(F("Maximum speed"));
  } else {
    Serial.print(1000.0 / interval, 1);
    Serial.println(F(" Hz"));
  }
  Serial.println();

  // Open or create file
  test.dataFile = SD.open(test.fileName, FILE_WRITE);
  if (!test.dataFile) {
    Serial.println(F("✗ Error opening file!"));
    return;
  }

  // Write CSV header if new file
  if (test.dataFile.size() == 0) {
    test.dataFile.println(F("Timestamp,Temp_C,Pressure_hPa,Altitude_m,GPS_Lat,GPS_Lon,GPS_Alt_m,GPS_Sats,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,State"));
    Serial.println(F("✓ Created new file with header"));
  } else {
    Serial.print(F("✓ Appending to existing file ("));
    Serial.print(test.dataFile.size());
    Serial.println(F(" bytes)"));
  }

  test.logging = true;
  Serial.println(F("✓ Logging started!\n"));
  Serial.println(F("Timestamp\tRecords\tBytes\tWrite(ms)\tFree(MB)\tUsed%"));
  Serial.println(F("----------------------------------------------------------------"));
}

void stopLogging() {
  if (!test.logging) return;

  test.logging = false;
  test.dataFile.close();

  unsigned long elapsedTime = millis() - test.startTime;

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("Logging Stopped - Summary"));
  Serial.println(F("========================================"));

  Serial.print(F("Mode: "));
  Serial.println(getModeString(test.mode));

  Serial.print(F("Duration: "));
  printDuration(elapsedTime);

  Serial.print(F("Records Written: "));
  Serial.println(test.recordsWritten);

  Serial.print(F("Bytes Written: "));
  printBytes(test.bytesWritten);

  Serial.print(F("Write Errors: "));
  Serial.println(test.writeErrors);

  if (test.writeCount > 0) {
    Serial.print(F("Write Time: Min="));
    Serial.print(test.minWriteTime);
    Serial.print(F("ms, Max="));
    Serial.print(test.maxWriteTime);
    Serial.print(F("ms, Avg="));
    Serial.print(test.totalWriteTime / test.writeCount);
    Serial.println(F("ms"));
  }

  if (elapsedTime > 0) {
    float recordsPerSec = (test.recordsWritten * 1000.0) / elapsedTime;
    float bytesPerSec = (test.bytesWritten * 1000.0) / elapsedTime;

    Serial.print(F("Average Rate: "));
    Serial.print(recordsPerSec, 2);
    Serial.print(F(" records/sec, "));
    Serial.print(bytesPerSec / 1024.0, 2);
    Serial.println(F(" KB/sec"));
  }

  updateCardInfo();
  Serial.print(F("Card Free Space: "));
  Serial.print(cardInfo.freeSpace);
  Serial.print(F(" MB ("));
  Serial.print(cardInfo.percentUsed, 1);
  Serial.println(F("% used)"));

  Serial.println(F("========================================\n"));
}

void writeDataRecord() {
  unsigned long writeStart = millis();

  // Simulate realistic telemetry data (matches actual CSV format)
  unsigned long timestamp = millis();
  float temp = 22.5 + random(-50, 50) / 10.0;
  float pressure = 1013.25 + random(-100, 100) / 10.0;
  float altitude = random(0, 10000) / 10.0;
  float lat = 40.712800 + random(-1000, 1000) / 1000000.0;
  float lon = -74.006000 + random(-1000, 1000) / 1000000.0;
  float gps_alt = altitude + random(-100, 100) / 10.0;
  int sats = random(4, 12);
  float accel_x = random(-100, 100) / 10.0;
  float accel_y = random(-100, 100) / 10.0;
  float accel_z = 9.81 + random(-50, 50) / 10.0;
  float gyro_x = random(-100, 100) / 10.0;
  float gyro_y = random(-100, 100) / 10.0;
  float gyro_z = random(-100, 100) / 10.0;
  const char* state = "FLIGHT";

  // Build CSV line
  String dataString = "";
  dataString += timestamp;
  dataString += ",";
  dataString += String(temp, 2);
  dataString += ",";
  dataString += String(pressure, 2);
  dataString += ",";
  dataString += String(altitude, 2);
  dataString += ",";
  dataString += String(lat, 6);
  dataString += ",";
  dataString += String(lon, 6);
  dataString += ",";
  dataString += String(gps_alt, 2);
  dataString += ",";
  dataString += sats;
  dataString += ",";
  dataString += String(accel_x, 2);
  dataString += ",";
  dataString += String(accel_y, 2);
  dataString += ",";
  dataString += String(accel_z, 2);
  dataString += ",";
  dataString += String(gyro_x, 2);
  dataString += ",";
  dataString += String(gyro_y, 2);
  dataString += ",";
  dataString += String(gyro_z, 2);
  dataString += ",";
  dataString += state;

  // Write to SD card
  if (test.dataFile) {
    size_t bytesWritten = test.dataFile.println(dataString);
    test.dataFile.flush();  // Force write to card

    if (bytesWritten > 0) {
      test.recordsWritten++;
      test.bytesWritten += bytesWritten;
    } else {
      test.writeErrors++;
    }
  } else {
    test.writeErrors++;
  }

  // Update timing statistics
  unsigned long writeTime = millis() - writeStart;
  if (writeTime < test.minWriteTime) test.minWriteTime = writeTime;
  if (writeTime > test.maxWriteTime) test.maxWriteTime = writeTime;
  test.totalWriteTime += writeTime;
  test.writeCount++;
}

void printStatus() {
  unsigned long elapsed = millis() - test.startTime;
  unsigned long avgWriteTime = test.writeCount > 0 ? test.totalWriteTime / test.writeCount : 0;

  Serial.print(elapsed / 1000);
  Serial.print(F("s\t\t"));
  Serial.print(test.recordsWritten);
  Serial.print(F("\t"));
  Serial.print(test.bytesWritten / 1024);
  Serial.print(F("K\t"));
  Serial.print(avgWriteTime);
  Serial.print(F("\t\t"));
  Serial.print(cardInfo.freeSpace);
  Serial.print(F("\t"));
  Serial.print(cardInfo.percentUsed, 1);
  Serial.println(F("%"));
}

void printProjection() {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("Capacity Projection"));
  Serial.println(F("========================================"));

  updateCardInfo();

  unsigned long elapsedTime = millis() - test.startTime;
  if (elapsedTime == 0 || test.bytesWritten == 0) {
    Serial.println(F("Not enough data yet for projection"));
    Serial.println(F("========================================\n"));
    return;
  }

  // Calculate current write rate
  float bytesPerSec = (test.bytesWritten * 1000.0) / elapsedTime;
  float kbPerSec = bytesPerSec / 1024.0;
  float mbPerHour = (bytesPerSec * 3600.0) / (1024.0 * 1024.0);

  Serial.print(F("Current Write Rate: "));
  Serial.print(kbPerSec, 2);
  Serial.print(F(" KB/s ("));
  Serial.print(mbPerHour, 2);
  Serial.println(F(" MB/hour)"));

  // Calculate time to fill card
  unsigned long freeBytes = cardInfo.freeSpace * 1024UL * 1024UL;
  unsigned long secondsToFill = freeBytes / bytesPerSec;

  Serial.print(F("Free Space: "));
  Serial.print(cardInfo.freeSpace);
  Serial.print(F(" MB ("));
  Serial.print(cardInfo.percentUsed, 1);
  Serial.println(F("% used)"));

  Serial.print(F("Time to Fill Card: "));
  printDuration(secondsToFill * 1000UL);

  // Calculate max flight times
  Serial.println();
  Serial.println(F("Maximum Flight Times:"));

  // At 1 Hz (preflight/postflight)
  float bytesPerRecord = (float)test.bytesWritten / test.recordsWritten;
  if (bytesPerRecord > 0) {
    unsigned long records1Hz = freeBytes / bytesPerRecord;
    unsigned long seconds1Hz = records1Hz;  // 1 record/sec
    Serial.print(F("  @ 1 Hz:  "));
    printDuration(seconds1Hz * 1000UL);

    // At 10 Hz (normal flight)
    unsigned long records10Hz = freeBytes / bytesPerRecord;
    unsigned long seconds10Hz = records10Hz / 10;
    Serial.print(F("  @ 10 Hz: "));
    printDuration(seconds10Hz * 1000UL);

    // At 20 Hz (high-speed)
    unsigned long records20Hz = freeBytes / bytesPerRecord;
    unsigned long seconds20Hz = records20Hz / 20;
    Serial.print(F("  @ 20 Hz: "));
    printDuration(seconds20Hz * 1000UL);
  }

  Serial.println(F("========================================\n"));
}

void updateCardInfo() {
  // Get volume information
  File root = SD.open("/");
  uint32_t volumesize = 0;

  // For FAT32, we can estimate based on cluster size
  // This is approximate - actual calculation is complex
  cardInfo.cardSize = 16000;  // 16GB nominal

  // Count free clusters (simplified - may not be 100% accurate)
  uint32_t freeBytes = 0;
  uint32_t totalBytes = cardInfo.cardSize * 1024UL * 1024UL;

  // Estimate by counting files (rough approximation)
  File entry;
  uint32_t usedBytes = 0;
  root = SD.open("/");
  while (entry = root.openNextFile()) {
    usedBytes += entry.size();
    entry.close();
  }
  root.close();

  cardInfo.usedSpace = usedBytes / (1024UL * 1024UL);
  cardInfo.freeSpace = cardInfo.cardSize - cardInfo.usedSpace;
  cardInfo.percentUsed = (cardInfo.usedSpace * 100.0) / cardInfo.cardSize;
}

void printCardInfo() {
  Serial.println(F("SD Card Information:"));
  Serial.print(F("  Card Size: "));
  Serial.print(cardInfo.cardSize);
  Serial.println(F(" MB"));

  Serial.print(F("  Used Space: "));
  Serial.print(cardInfo.usedSpace);
  Serial.print(F(" MB ("));
  Serial.print(cardInfo.percentUsed, 1);
  Serial.println(F("%)"));

  Serial.print(F("  Free Space: "));
  Serial.print(cardInfo.freeSpace);
  Serial.println(F(" MB"));
  Serial.println();
}

void deleteTestFiles() {
  Serial.println(F("Deleting test files..."));

  const char* testFiles[] = {
    "log_1hz.csv",
    "log_10hz.csv",
    "log_20hz.csv",
    "log_max.csv"
  };

  int deletedCount = 0;
  for (int i = 0; i < 4; i++) {
    if (SD.exists(testFiles[i])) {
      File f = SD.open(testFiles[i]);
      uint32_t size = f.size();
      f.close();

      if (SD.remove(testFiles[i])) {
        Serial.print(F("  ✓ Deleted "));
        Serial.print(testFiles[i]);
        Serial.print(F(" ("));
        Serial.print(size / 1024);
        Serial.println(F(" KB)"));
        deletedCount++;
      }
    }
  }

  if (deletedCount == 0) {
    Serial.println(F("  No test files found"));
  } else {
    Serial.print(F("\n✓ Deleted "));
    Serial.print(deletedCount);
    Serial.println(F(" file(s)"));
  }

  updateCardInfo();
  Serial.println();
}

void printDuration(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  if (days > 0) {
    Serial.print(days);
    Serial.print(F(" days, "));
    Serial.print(hours % 24);
    Serial.println(F(" hours"));
  } else if (hours > 0) {
    Serial.print(hours);
    Serial.print(F(" hours, "));
    Serial.print(minutes % 60);
    Serial.println(F(" minutes"));
  } else if (minutes > 0) {
    Serial.print(minutes);
    Serial.print(F(" minutes, "));
    Serial.print(seconds % 60);
    Serial.println(F(" seconds"));
  } else {
    Serial.print(seconds);
    Serial.println(F(" seconds"));
  }
}

void printBytes(unsigned long bytes) {
  if (bytes < 1024) {
    Serial.print(bytes);
    Serial.println(F(" bytes"));
  } else if (bytes < 1024 * 1024) {
    Serial.print(bytes / 1024.0, 2);
    Serial.println(F(" KB"));
  } else {
    Serial.print(bytes / (1024.0 * 1024.0), 2);
    Serial.println(F(" MB"));
  }
}

const char* getModeString(TestMode mode) {
  switch (mode) {
    case MODE_1HZ: return "1 Hz (Preflight/Postflight)";
    case MODE_10HZ: return "10 Hz (Normal Flight)";
    case MODE_20HZ: return "20 Hz (High-Speed)";
    case MODE_MAX_SPEED: return "Maximum Speed";
    default: return "Idle";
  }
}

void printMenu() {
  Serial.println(F("========================================"));
  Serial.println(F("Command Menu"));
  Serial.println(F("========================================"));
  Serial.println(F("Logging Tests:"));
  Serial.println(F("  1 - Start 1 Hz logging (preflight/postflight)"));
  Serial.println(F("  2 - Start 10 Hz logging (normal flight)"));
  Serial.println(F("  3 - Start 20 Hz logging (high-speed)"));
  Serial.println(F("  4 - Start max speed logging (stress test)"));
  Serial.println();
  Serial.println(F("Control:"));
  Serial.println(F("  s - Stop logging"));
  Serial.println(F("  i - Show card info"));
  Serial.println(F("  d - Delete test files"));
  Serial.println(F("  h - Show this menu"));
  Serial.println(F("========================================\n"));
}
