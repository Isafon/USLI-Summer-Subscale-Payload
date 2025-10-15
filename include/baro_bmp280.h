#ifndef BARO_BMP280_H
#define BARO_BMP280_H

#include <Arduino.h>

struct BaroData {
  float temperature;
  float pressure;
  float altitude;
  bool dataValid;
};

bool initBaro();
bool readBaro(BaroData &data);

#endif