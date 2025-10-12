#include "rtc_pcf8523.h"

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

// Helper function to convert decimal to BCD
static uint8_t dec2bcd(uint8_t val) {
  return val + 6 * (val / 10);
}

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

bool setRTC(uint16_t year, uint8_t month, uint8_t day, 
            uint8_t hour, uint8_t minute, uint8_t second) {
  
  // Validate input
  if (year < 2000 || year > 2099 || month < 1 || month > 12 ||
      day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) {
    Serial.println(F("Invalid date/time values"));
    return false;
  }
  
  // Convert to BCD
  uint8_t yearBCD = dec2bcd(year - 2000);
  uint8_t monthBCD = dec2bcd(month);
  uint8_t dayBCD = dec2bcd(day);
  uint8_t hourBCD = dec2bcd(hour);
  uint8_t minuteBCD = dec2bcd(minute);
  uint8_t secondBCD = dec2bcd(second);
  
  // Write to RTC
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire.write(PCF8523_SECONDS);
  Wire.write(secondBCD);
  Wire.write(minuteBCD);
  Wire.write(hourBCD);     // 24-hour format
  Wire.write(dayBCD);
  Wire.write(0);           // weekday (can be 0)
  Wire.write(monthBCD);
  Wire.write(yearBCD);
  
  if (Wire.endTransmission() != 0) {
    Serial.println(F("Failed to set RTC time"));
    return false;
  }
  
  Serial.println(F("RTC time set successfully"));
  return true;
}

void printDateTime(const DateTime &dt) {
  if (!dt.dataValid) {
    Serial.println(F("RTC: No valid data"));
    return;
  }
  
  Serial.print(F("RTC: "));
  Serial.print(dt.year);
  Serial.print(F("-"));
  if (dt.month < 10) Serial.print(F("0"));
  Serial.print(dt.month);
  Serial.print(F("-"));
  if (dt.day < 10) Serial.print(F("0"));
  Serial.print(dt.day);
  Serial.print(F(" "));
  if (dt.hour < 10) Serial.print(F("0"));
  Serial.print(dt.hour);
  Serial.print(F(":"));
  if (dt.minute < 10) Serial.print(F("0"));
  Serial.print(dt.minute);
  Serial.print(F(":"));
  if (dt.second < 10) Serial.print(F("0"));
  Serial.println(dt.second);
}

void getTimestamp(char* buffer, size_t bufferSize) {
  DateTime dt;
  if (readRTC(dt) && dt.dataValid) {
    snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d",
             dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
  } else {
    snprintf(buffer, bufferSize, "0000-00-00 00:00:00");
  }
}

bool isRTCConnected() {
  Wire.beginTransmission(PCF8523_ADDRESS);
  return (Wire.endTransmission() == 0);
}

uint32_t getUnixTime(const DateTime &dt) {
  if (!dt.dataValid) return 0;
  
  // Simple Unix timestamp calculation (seconds since Jan 1, 2000)
  // For accurate Unix time (since 1970), add 946684800 to result
  uint32_t y = dt.year - 2000;
  uint32_t d = dt.day;
  
  // Add days for complete years
  d += 365 * y + (y + 3) / 4;  // Leap years
  
  // Add days for complete months
  static const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  for (uint8_t i = 1; i < dt.month; i++) {
    d += daysInMonth[i - 1];
  }
  
  // Adjust for leap year
  if (dt.month > 2 && (y % 4 == 0)) {
    d++;
  }
  
  // Convert to seconds
  uint32_t seconds = ((d - 1) * 24UL + dt.hour) * 3600UL + dt.minute * 60UL + dt.second;
  
  return seconds;
}

