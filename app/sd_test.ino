/*
 * SD Card Test - Simple diagnostic sketch
 */

#include <SD.h>
#include <SPI.h>

// SD Card CS pin
#define SD_CS_PIN 9

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== SD Card Diagnostic Test ==="));
  
  // Test 1: Basic SD.begin()
  Serial.println(F("Testing SD.begin()..."));
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("❌ SD.begin() failed!"));
    Serial.println(F("Possible issues:"));
    Serial.println(F("1. SD card not formatted as FAT32"));
    Serial.println(F("2. SD card not inserted properly"));
    Serial.println(F("3. Wiring issue"));
    Serial.println(F("4. SD card module defective"));
    Serial.println(F("5. Power issue"));
  } else {
    Serial.println(F("✅ SD.begin() successful!"));
    
    // Test 2: List files
    Serial.println(F("\nListing files on SD card:"));
    File root = SD.open("/");
    if (root) {
      while (File entry = root.openNextFile()) {
        Serial.print(F("  "));
        Serial.print(entry.name());
        Serial.print(F(" - "));
        Serial.print(entry.size());
        Serial.println(F(" bytes"));
        entry.close();
      }
      root.close();
    }
    
    // Test 3: Write test file
    Serial.println(F("\nTesting file write..."));
    File testFile = SD.open("test.txt", FILE_WRITE);
    if (testFile) {
      testFile.println("SD card test successful!");
      testFile.println("Timestamp: " + String(millis()));
      testFile.close();
      Serial.println(F("✅ File write successful!"));
    } else {
      Serial.println(F("❌ File write failed!"));
    }
    
    // Test 4: Read test file
    Serial.println(F("\nTesting file read..."));
    File readFile = SD.open("test.txt");
    if (readFile) {
      Serial.println(F("✅ File read successful!"));
      Serial.println(F("File contents:"));
      while (readFile.available()) {
        Serial.write(readFile.read());
      }
      readFile.close();
    } else {
      Serial.println(F("❌ File read failed!"));
    }
  }
  
  Serial.println(F("\n=== Test Complete ==="));
}

void loop() {
  // Blink LED to show system is alive
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}
