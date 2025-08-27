#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "gps.h"

// GPS wiring (We are using Arduino Nano RX0/TX1)
const int RXPin = 0, TXPin = 1;
const uint32_t GPSBaud = 4800;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void initGPS() {
  ss.begin(GPSBaud);
  Serial.println(F("GPS setup complete"));
}

void updateGPS() {
  // Feed data from GPS module
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }
}
