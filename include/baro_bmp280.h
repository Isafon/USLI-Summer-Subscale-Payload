#ifndef BARO_BMP280_H
#define BARO_BMP280_H

#include <Arduino.h>
#include <Adafruit_BMP280.h>

// Barometer data structure
struct BaroData {
  float pressure;      // hPa
  float temperature;   // °C
  float altitude;      // meters
  bool dataValid;
};

// Function prototypes
bool initBaro();
bool readBaro(BaroData &data);
void printBaroData(const BaroData &data);
bool isBaroConnected();
float calculateAltitude(float pressure, float seaLevelPressure = 1013.25);

#endif // BARO_BMP280_H