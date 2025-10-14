#ifndef RTC_PCF8523_H
#define RTC_PCF8523_H

#include <Arduino.h>

struct DateTime {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  bool dataValid;
};

bool initRTC();
bool readRTC(DateTime &dt);

#endif

