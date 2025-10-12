#ifndef RTC_PCF8523_H
#define RTC_PCF8523_H

#include <Arduino.h>
#include <Wire.h>

// RTC DateTime structure for timestamp
struct DateTime {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  bool dataValid;
};

// Function prototypes
bool initRTC();
bool readRTC(DateTime &dt);
bool setRTC(uint16_t year, uint8_t month, uint8_t day, 
            uint8_t hour, uint8_t minute, uint8_t second);
void printDateTime(const DateTime &dt);
void getTimestamp(char* buffer, size_t bufferSize);
bool isRTCConnected();
uint32_t getUnixTime(const DateTime &dt);

#endif // RTC_PCF8523_H

