/*
 * I2C Scanner
 *
 * This sketch scans the I2C bus to detect all connected devices.
 * Very useful for troubleshooting I2C connections!
 *
 * Common I2C Addresses for this project:
 * - 0x68/0x69: ICM-20948 IMU
 * - 0x76/0x77: BMP280 Barometer
 *
 * Hardware Connections:
 * - SDA -> A4 (Arduino Nano)
 * - SCL -> A5 (Arduino Nano)
 * - All I2C devices share the same SDA/SCL lines
 * - Each device needs VCC and GND
 *
 * Expected Output:
 * - List of all detected I2C addresses
 * - Identification of known devices
 */

#include <Arduino.h>
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("\n========================================"));
  Serial.println(F("I2C Bus Scanner"));
  Serial.println(F("========================================"));
  Serial.println(F("Scanning I2C bus (addresses 0x01-0x7F)..."));
  Serial.println();
}

void loop() {
  byte error, address;
  int deviceCount = 0;

  Serial.println(F("Scanning..."));
  Serial.println(F("----------------------------------------"));

  for (address = 1; address < 127; address++) {
    // Try to communicate with device at this address
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      // Device found!
      Serial.print(F("✓ I2C device found at address 0x"));
      if (address < 16) Serial.print(F("0"));
      Serial.print(address, HEX);
      Serial.print(F("  ("));

      // Identify known devices
      identifyDevice(address);

      Serial.println(F(")"));
      deviceCount++;
    }
    else if (error == 4) {
      Serial.print(F("✗ Unknown error at address 0x"));
      if (address < 16) Serial.print(F("0"));
      Serial.println(address, HEX);
    }
  }

  Serial.println(F("----------------------------------------"));

  if (deviceCount == 0) {
    Serial.println(F("✗ No I2C devices found!"));
    Serial.println();
    Serial.println(F("Troubleshooting tips:"));
    Serial.println(F("  1. Check SDA -> A4 connection"));
    Serial.println(F("  2. Check SCL -> A5 connection"));
    Serial.println(F("  3. Verify device power (VCC and GND)"));
    Serial.println(F("  4. Check if pullup resistors are present"));
    Serial.println(F("     (most modules have them built-in)"));
    Serial.println(F("  5. Verify device voltage compatibility"));
    Serial.println(F("     - ICM-20948: 3.3V ONLY!"));
    Serial.println(F("     - BMP280: 3.3V or 5V (check your module)"));
  } else {
    Serial.print(F("✓ Found "));
    Serial.print(deviceCount);
    Serial.println(F(" device(s)"));

    // Check if expected devices are present
    Serial.println();
    Serial.println(F("Expected Devices:"));
    checkExpectedDevice(0x68, "ICM-20948 IMU (AD0=0)");
    checkExpectedDevice(0x69, "ICM-20948 IMU (AD0=1)");
    checkExpectedDevice(0x76, "BMP280 Barometer (SDO=0)");
    checkExpectedDevice(0x77, "BMP280 Barometer (SDO=1)");
  }

  Serial.println(F("========================================\n"));

  // Scan again in 5 seconds
  delay(5000);
}

void identifyDevice(byte address) {
  switch (address) {
    case 0x68:
      Serial.print(F("ICM-20948 IMU (AD0=0) or MPU6050"));
      break;
    case 0x69:
      Serial.print(F("ICM-20948 IMU (AD0=1)"));
      break;
    case 0x76:
      Serial.print(F("BMP280/BME280 (SDO=0)"));
      break;
    case 0x77:
      Serial.print(F("BMP280/BME280 (SDO=1)"));
      break;
    case 0x48:
      Serial.print(F("ADS1115 ADC or other"));
      break;
    case 0x50:
      Serial.print(F("EEPROM or other"));
      break;
    default:
      Serial.print(F("Unknown device"));
      break;
  }
}

void checkExpectedDevice(byte address, const char* deviceName) {
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();

  Serial.print(F("  0x"));
  if (address < 16) Serial.print(F("0"));
  Serial.print(address, HEX);
  Serial.print(F(" - "));
  Serial.print(deviceName);
  Serial.print(F(": "));

  if (error == 0) {
    Serial.println(F("✓ FOUND"));
  } else {
    Serial.println(F("✗ NOT FOUND"));
  }
}
