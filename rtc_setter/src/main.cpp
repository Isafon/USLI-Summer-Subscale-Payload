/*
 * RTC Time Setter Utility
 * 
 * This utility allows you to set the RTC time independently.
 * Compile and upload this file, open Serial Monitor, and follow the prompts.
 * 
 * Wiring:
 * - RTC PCF8523 VCC -> 3.3V
 * - RTC PCF8523 GND -> GND
 * - RTC PCF8523 SDA -> A4 (Arduino Nano)
 * - RTC PCF8523 SCL -> A5 (Arduino Nano)
 */

#include <Arduino.h>
#include <Wire.h>
#include "rtc_pcf8523.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println(F("=== RTC Time Setter Utility ==="));
  Serial.println(F("This utility will set the RTC time."));
  Serial.println();
  
  // Initialize I2C
  Wire.begin();
  Serial.println(F("✓ I2C initialized"));
  
  // Initialize RTC
  if (!initRTC()) {
    Serial.println(F("✗ Failed to initialize RTC!"));
    Serial.println(F("Check wiring: SDA->A4, SCL->A5, VCC->3.3V, GND->GND"));
    while (1) delay(1000);
  }
  Serial.println(F("✓ RTC initialized successfully"));
  
  // Read current time
  DateTime currentTime;
  if (readRTC(currentTime)) {
    Serial.print(F("Current RTC time: "));
    Serial.print(currentTime.year); Serial.print(F("-"));
    Serial.print(currentTime.month); Serial.print(F("-"));
    Serial.print(currentTime.day); Serial.print(F(" "));
    Serial.print(currentTime.hour); Serial.print(F(":"));
    Serial.print(currentTime.minute); Serial.print(F(":"));
    Serial.println(currentTime.second);
  } else {
    Serial.println(F("Could not read current RTC time"));
  }
  
  Serial.println();
  Serial.println(F("=== AUTO SET MODE ==="));
  Serial.println(F("Setting RTC to fixed time..."));
  
  // EDIT THESE VALUES to set your desired time

  int year = 2025;
  int month = 10;
  int day = 29;
  int hour = 19;

  int minute = 4;
  int second = 45;
  
  Serial.print(F("Setting RTC to: "));
  Serial.print(year); Serial.print(F("-"));
  Serial.print(month); Serial.print(F("-"));
  Serial.print(day); Serial.print(F(" "));
  Serial.print(hour); Serial.print(F(":"));
  Serial.print(minute); Serial.print(F(":"));
  Serial.println(second);
  
  if (setRTC(year, month, day, hour, minute, second)) {
    Serial.println(F("✓ RTC time set successfully!"));
    
    // Wait for RTC to update
    delay(100);
    
    // Verify the time was set
    DateTime verifyTime;
    if (readRTC(verifyTime)) {
      Serial.print(F("Verified RTC time: "));
      Serial.print(verifyTime.year); Serial.print(F("-"));
      Serial.print(verifyTime.month); Serial.print(F("-"));
      Serial.print(verifyTime.day); Serial.print(F(" "));
      Serial.print(verifyTime.hour); Serial.print(F(":"));
      Serial.print(verifyTime.minute); Serial.print(F(":"));
      Serial.println(verifyTime.second);
    }
  } else {
    Serial.println(F("✗ Failed to set RTC time!"));
  }
  
  Serial.println(F("Done! Upload your main program now."));
}

void loop() {
  // Just show current time every 5 seconds
  static unsigned long lastTimeDisplay = 0;
  if (millis() - lastTimeDisplay > 5000) {
    DateTime currentTime;
    if (readRTC(currentTime)) {
      Serial.print(F("Current RTC: "));
      Serial.print(currentTime.year); Serial.print(F("-"));
      Serial.print(currentTime.month); Serial.print(F("-"));
      Serial.print(currentTime.day); Serial.print(F(" "));
      Serial.print(currentTime.hour); Serial.print(F(":"));
      Serial.print(currentTime.minute); Serial.print(F(":"));
      Serial.println(currentTime.second);
    }
    lastTimeDisplay = millis();
  }
}
