#include <Arduino.h>
#include <Wire.h>
#include "rtc_pcf8523.h"

// Test configuration
#define TIMESTAMP_INTERVAL 1000  // Print timestamp every 1 second
#define DEMO_DATA_LOGGING true   // Demonstrate data logging with timestamps

unsigned long lastTimestamp = 0;
int dataCounter = 0;

// I2C Scanner function
void scanI2C() {
  Serial.println(F("\n--- I2C Scanner ---"));
  Serial.println(F("Scanning I2C bus..."));
  
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("Found device at address 0x"));
      if (i < 16) Serial.print("0");
      Serial.print(i, HEX);
      
      if (i == 0x68) {
        Serial.print(F(" (PCF8523 RTC!)"));
      }
      Serial.println();
      count++;
      delay(1);
    }
  }
  
  if (count == 0) {
    Serial.println(F("No I2C devices found!"));
    Serial.println(F("Check wiring:"));
    Serial.println(F("  SDA -> A4"));
    Serial.println(F("  SCL -> A5"));
    Serial.println(F("  VCC -> 5V or 3.3V"));
    Serial.println(F("  GND -> GND"));
  } else {
    Serial.print(F("Found "));
    Serial.print(count);
    Serial.println(F(" device(s)"));
  }
  Serial.println(F("-------------------\n"));
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for serial port to connect
  }
  
  Serial.println(F("\n========================================"));
  Serial.println(F("  PCF8523 RTC Driver Test Program"));
  Serial.println(F("========================================\n"));
  
  // Initialize I2C first
  Wire.begin();
  delay(100);
  
  // Scan for I2C devices
  scanI2C();
  
  // Initialize RTC
  if (!initRTC()) {
    Serial.println(F("ERROR: Failed to initialize RTC!"));
    Serial.println(F("Please check connections and reset."));
    Serial.println(F("\nRunning I2C scan again..."));
    scanI2C();
    Serial.println(F("Halted. Press RESET to try again."));
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println();
  
  // Check if RTC needs to be set
  DateTime currentTime;
  if (readRTC(currentTime)) {
    Serial.println(F("Current RTC time:"));
    printDateTime(currentTime);
    
    // Check if time seems reasonable (not 12:00-12:05 which is the default)
    bool isDefaultTime = (currentTime.hour == 12 && currentTime.minute < 5);
    
    if (currentTime.year < 2024 || isDefaultTime) {
      Serial.println(F("\nRTC time appears to be unset or default."));
      Serial.println(F("Setting RTC to current time: 2025-10-12 16:58:10"));
      
      // Set to current date and time (MODIFY THIS to match actual time!)
      // Format: setRTC(year, month, day, hour, minute, second)
      setRTC(2025, 10, 12, 16, 58, 10);
      
      delay(100);
      if (readRTC(currentTime)) {
        Serial.println(F("\nNew RTC time:"));
        printDateTime(currentTime);
      }
    }
  } else {
    Serial.println(F("WARNING: Could not read RTC time initially"));
    Serial.println(F("Attempting to set time anyway..."));
    setRTC(2025, 10, 12, 12, 0, 0);
    delay(100);
  }
  
  Serial.println(F("\n========================================"));
  Serial.println(F("Starting continuous timestamp logging..."));
  Serial.println(F("========================================\n"));
  
  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Print timestamp at regular intervals
  if (currentMillis - lastTimestamp >= TIMESTAMP_INTERVAL) {
    lastTimestamp = currentMillis;
    
    // Read current time from RTC
    DateTime dt;
    if (readRTC(dt)) {
      
      if (DEMO_DATA_LOGGING) {
        // Demonstrate data logging with timestamp
        char timestamp[32];
        getTimestamp(timestamp, sizeof(timestamp));
        
        // Simulate sensor data
        float sensorValue = 20.5 + sin(dataCounter * 0.1) * 5.0;
        
        Serial.print(F("["));
        Serial.print(timestamp);
        Serial.print(F("] "));
        Serial.print(F("Data #"));
        Serial.print(dataCounter);
        Serial.print(F(": Sensor = "));
        Serial.print(sensorValue, 2);
        Serial.print(F(" | Unix = "));
        Serial.println(getUnixTime(dt));
        
        dataCounter++;
      } else {
        // Just print the timestamp
        printDateTime(dt);
      }
      
    } else {
      Serial.println(F("ERROR: Failed to read RTC"));
      Serial.println(F("  Checking I2C connection..."));
      if (!isRTCConnected()) {
        Serial.println(F("  RTC is NOT responding on I2C bus!"));
        Serial.println(F("  Running I2C scan..."));
        scanI2C();
      }
    }
  }
  
  // Check for serial commands
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Clear input buffer
    while (Serial.available() > 0) {
      Serial.read();
    }
    
    switch (cmd) {
      case 'i':
      case 'I':
        // Run I2C scan
        scanI2C();
        break;
        
      case 't':
      case 'T': {
        // Read and print current time
        DateTime dt;
        Serial.println(F("\n--- Current Time ---"));
        if (readRTC(dt)) {
          printDateTime(dt);
          Serial.print(F("Unix Time (since 2000): "));
          Serial.println(getUnixTime(dt));
        } else {
          Serial.println(F("Failed to read RTC"));
        }
        Serial.println();
        break;
      }
      
      case 's':
      case 'S':
        // Set time (example: set to compile time or a specific time)
        Serial.println(F("\n--- Setting RTC Time ---"));
        Serial.println(F("Setting to: 2025-10-12 17:04:00"));
        if (setRTC(2025, 10, 12, 17, 4, 0)) {
          Serial.println(F("Time set successfully"));
          DateTime dt;
          if (readRTC(dt)) {
            Serial.print(F("New time: "));
            printDateTime(dt);
          }
        }
        Serial.println();
        break;
        
      case 'c':
      case 'C':
        // Check RTC connection
        Serial.println(F("\n--- RTC Connection Check ---"));
        if (isRTCConnected()) {
          Serial.println(F("RTC is connected (I2C address 0x68)"));
        } else {
          Serial.println(F("RTC not found!"));
        }
        Serial.println();
        break;
        
      case 'h':
      case 'H':
        // Print help
        Serial.println(F("\n========================================"));
        Serial.println(F("  RTC Test Commands"));
        Serial.println(F("========================================"));
        Serial.println(F("  I - Scan I2C bus for devices"));
        Serial.println(F("  T - Read and display current time"));
        Serial.println(F("  S - Set RTC time to example value"));
        Serial.println(F("  C - Check RTC connection"));
        Serial.println(F("  H - Show this help menu"));
        Serial.println(F("========================================\n"));
        break;
        
      default:
        Serial.println(F("Unknown command. Press 'H' for help."));
        break;
    }
  }
}
