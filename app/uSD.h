#ifndef USD_H
#define USD_H

#include <Arduino.h>  // for String
#include <SD.h>       // for File handling

// Initialize the SD card
bool initSD();

// Append a line of telemetry data to flight.txt
bool logData(const String &data);

#endif // USD_H
