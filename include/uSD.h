#ifndef USD_H
#define USD_H

#include "rtc_pcf8523.h"
#include "baro_bmp280.h"

bool initSD();
bool startLogging(const char* fileName);
bool stopLogging();
bool writeData(const DateTime& dt, const BaroData& data);
bool writeData(const DateTime& dt, const char* event, const char* message);
bool deleteFile(const char* fileName);
bool isLoggingActive();
const char* getCurrentFileName();

#endif
