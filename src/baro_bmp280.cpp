#include "baro_bmp280.h"
#include "config.h"
#include <SPI.h>

// Create BMP280 object - SOFTWARE SPI for reliability
Adafruit_BMP280 bmp(BARO_CS_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN);

// Sea level pressure for altitude calculation
static float seaLevelPressure = 1013.25; // hPa

bool initBaro() {
  delay(50);
  
  if (!bmp.begin()) {
    delay(100);
    if (!bmp.begin()) {
      return false;
    }
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  
  delay(100);
  return true;
}

bool readBaro(BaroData &data) {
  float pressure = bmp.readPressure();
  float temperature = bmp.readTemperature();
  
  if (isnan(pressure) || isnan(temperature)) {
    data.dataValid = false;
    return false;
  }
  
  data.pressure = pressure / 100.0F;
  data.temperature = temperature;
  data.altitude = calculateAltitude(data.pressure, seaLevelPressure);
  data.dataValid = true;
  
  return true;
}

float calculateAltitude(float pressure, float seaLevelPressure) {
  return 44330.0 * (1.0 - pow(pressure / seaLevelPressure, 0.1903));
}
