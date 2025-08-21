#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// GPS wiring (We are using Arduino Nano RX0/TX1)
static const int RXPin = 0, TXPin = 1;
static const uint32_t GPSBaud = 4800;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Serial.println(F("GPS setup complete "));
}

void loop() {
  // Feed data from GPS module
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  // Only print when data is updated
  if (gps.location.isUpdated()) {
    Serial.print(F("Lat: ")); Serial.println(gps.location.lat(), 6);
    Serial.print(F("Lng: ")); Serial.println(gps.location.lng(), 6);
  }

  if (gps.altitude.isUpdated()) {
    Serial.print(F("Alt (m): ")); Serial.println(gps.altitude.meters());
  }

  if (gps.date.isUpdated() && gps.time.isUpdated()) {
    Serial.print(F("Date: "));
    Serial.print(gps.date.month()); Serial.print("/");
    Serial.print(gps.date.day()); Serial.print("/");
    Serial.println(gps.date.year());

    Serial.print(F("Time: "));
    Serial.print(gps.time.hour()); Serial.print(":");
    Serial.print(gps.time.minute()); Serial.print(":");
    Serial.println(gps.time.second());
  }

  if (gps.satellites.isUpdated()) {
    Serial.print(F("Satellites: ")); Serial.println(gps.satellites.value());
  }
}
