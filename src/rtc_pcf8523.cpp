#include "rtc_pcf8523.h"
#include <Wire.h>

// PCF8523 I2C address
#define PCF8523_ADDRESS 0x68

// PCF8523 register addresses
#define PCF8523_CONTROL_1       0x00
#define PCF8523_CONTROL_2       0x01
#define PCF8523_CONTROL_3       0x02
#define PCF8523_SECONDS         0x03
#define PCF8523_MINUTES         0x04
#define PCF8523_HOURS           0x05
#define PCF8523_DAYS            0x06
#define PCF8523_WEEKDAYS        0x07
#define PCF8523_MONTHS          0x08
#define PCF8523_YEARS           0x09

// Helper function to convert BCD to decimal
static uint8_t bcd2dec(uint8_t val) {
  return val - 6 * (val >> 4);
}

// Unused function removed

// Read a register from PCF8523
static uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom(PCF8523_ADDRESS, 1);
  return Wire.read();
}

// Write a register to PCF8523
static void writeRegister(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool initRTC() {
  Wire.begin();
  Wire.beginTransmission(PCF8523_ADDRESS);
  if (Wire.endTransmission() != 0) return false;
  
  uint8_t control1 = readRegister(PCF8523_CONTROL_1);
  if (control1 & 0x20) {
    writeRegister(PCF8523_CONTROL_1, control1 & ~0x20);
    delay(10);
  }
  
  return true;
}

bool readRTC(DateTime &dt) {
  // Set register pointer to seconds register
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire.write(PCF8523_SECONDS);
  uint8_t error = Wire.endTransmission();
  if (error != 0) {
    dt.dataValid = false;
    Serial.print(F("  I2C Error Code: "));
    Serial.println(error);
    Serial.println(F("  (1=data too long, 2=NACK on addr, 3=NACK on data, 4=other)"));
    return false;
  }
  
  // Read 7 bytes (seconds, minutes, hours, days, weekdays, months, years)
  Wire.requestFrom(PCF8523_ADDRESS, 7);
  
  uint8_t available = Wire.available();
  if (available < 7) {
    dt.dataValid = false;
    Serial.print(F("  Expected 7 bytes, got: "));
    Serial.println(available);
    return false;
  }
  
  uint8_t seconds = Wire.read();
  uint8_t minutes = Wire.read();
  uint8_t hours = Wire.read();
  uint8_t days = Wire.read();
  Wire.read(); // weekdays (not used)
  uint8_t months = Wire.read();
  uint8_t years = Wire.read();
  
  // Convert BCD to decimal
  dt.second = bcd2dec(seconds & 0x7F);  // Mask out oscillator stop flag
  dt.minute = bcd2dec(minutes & 0x7F);
  dt.hour = bcd2dec(hours & 0x3F);      // Mask out AMPM and 12/24 bits
  dt.day = bcd2dec(days & 0x3F);
  dt.month = bcd2dec(months & 0x1F);
  dt.year = 2000 + bcd2dec(years);
  
  // Validate data ranges
  if (dt.second > 59 || dt.minute > 59 || dt.hour > 23 ||
      dt.day < 1 || dt.day > 31 || dt.month < 1 || dt.month > 12) {
    dt.dataValid = false;
    return false;
  }
  
  dt.dataValid = true;
  return true;
}

// Helper function to convert decimal to BCD
static uint8_t dec2bcd(uint8_t val) {
  return val + 6 * (val / 10);
}

// Set RTC time
bool setRTC(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  // Validate input ranges
  if (second > 59 || minute > 59 || hour > 23 ||
      day < 1 || day > 31 || month < 1 || month > 12 ||
      year < 2000 || year > 2099) {
    return false;
  }
  
  // Convert to BCD
  uint8_t seconds_bcd = dec2bcd(second);
  uint8_t minutes_bcd = dec2bcd(minute);
  uint8_t hours_bcd = dec2bcd(hour);
  uint8_t days_bcd = dec2bcd(day);
  uint8_t months_bcd = dec2bcd(month);
  uint8_t years_bcd = dec2bcd(year - 2000);
  
  // Write time registers
  writeRegister(PCF8523_SECONDS, seconds_bcd);
  writeRegister(PCF8523_MINUTES, minutes_bcd);
  writeRegister(PCF8523_HOURS, hours_bcd);
  writeRegister(PCF8523_DAYS, days_bcd);
  writeRegister(PCF8523_MONTHS, months_bcd);
  writeRegister(PCF8523_YEARS, years_bcd);
  
  return true;
}

// Unused functions removed to save memory

