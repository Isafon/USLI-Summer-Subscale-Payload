#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "baro_bmp280.h"

#define BMP_CS   3
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK  13

#define SEA_LEVEL_PRESSURE 1013.25  // hPa
#define N 5                         // median filter window size

Adafruit_BMP280 bmp; // I2C mode (default)

// Median filter buffer
float altBuf[N];
int altIndex = 0;

// Compute altitude from pressure
float calculateAltitude(float pressure_hPa, float seaLevel_hPa) {
  return 44330.0 * (1.0 - pow(pressure_hPa / seaLevel_hPa, 0.1903));
}

// Median filter
float filterAltitude(float newAlt) {
  altBuf[altIndex++ % N] = newAlt;
  float sorted[N];
  memcpy(sorted, altBuf, sizeof(sorted));
  for (int i = 0; i < N - 1; i++) {
    for (int j = i + 1; j < N; j++) {
      if (sorted[j] < sorted[i]) {
        float tmp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = tmp;
      }
    }
  }
  return sorted[N / 2];
}

// Plausibility check
bool validReading(float temp, float press) {
  if (isnan(temp) || isnan(press)) return false;
  if (temp < -40 || temp > 85) return false;
  if (press < 300 || press > 1100) return false;
  return true;
}

bool initBaro() {
  Serial.println(F("Initializing BMP280 (I2C mode)..."));
  
  if (!bmp.begin()) {
    Serial.println(F("✗ Failed to initialize BMP280 via I2C!"));
    Serial.println(F("Check wiring:"));
    Serial.println(F("  SDA -> A4"));
    Serial.println(F("  SCL -> A5"));
    Serial.println(F("  VCC -> 3.3V or 5V"));
    Serial.println(F("  GND -> GND"));
    return false;
  }

  Serial.println(F("✓ BMP280 initialized successfully!"));

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println(F("Configuration complete."));
  return true;
}

bool readBaro(BaroData &data) {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Pa → hPa

  // Validate reading
  if (!validReading(temperature, pressure)) {
    delay(10);
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0F;
    if (!validReading(temperature, pressure)) {
      return false;
    }
  }

  float altitude = calculateAltitude(pressure, SEA_LEVEL_PRESSURE);
  altitude = filterAltitude(altitude);

  data.temperature = temperature;
  data.pressure = pressure;
  data.altitude = altitude;

  return true;
}