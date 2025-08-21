#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// GPS wiring constants
extern const int RXPin;
extern const int TXPin;
extern const uint32_t GPSBaud;

// GPS objects
extern TinyGPSPlus gps;
extern SoftwareSerial ss;

// Standard Arduino functions
void setup();
void loop();

#endif // GPS_H