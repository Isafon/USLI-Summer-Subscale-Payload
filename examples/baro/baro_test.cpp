#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#define BMP_CS   3
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK  13

#define TEST_INTERVAL 500
#define SEA_LEVEL_PRESSURE 1013.25

Adafruit_BMP280 bmp(BMP_CS, SPI_MOSI, SPI_MISO, SPI_SCK);

// Statistics tracking
float minPressure = 999999, maxPressure = 0;
float minTemp = 999999, maxTemp = -999999;
float minAltitude = 999999, maxAltitude = -999999;
int readingCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("========================================"));
  Serial.println(F("BMP280 Barometer SPI Test"));
  Serial.println(F("========================================"));

  Serial.println(F("Initializing BMP280 (SPI mode)..."));
  if (!bmp.begin()) {
    Serial.println(F("✗ Failed to initialize BMP280 via SPI! Check wiring."));
    while (1) delay(1000);
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
  Serial.println(F("Time(ms)\tTemp(°C)\tPressure(hPa)\tAltitude(m)"));
}

void loop() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  float altitude = bmp.readAltitude(SEA_LEVEL_PRESSURE);

  if (isnan(temperature) || isnan(pressure)) {
    Serial.println(F("✗ Sensor read error!"));
    delay(TEST_INTERVAL);
    return;
  }

  readingCount++;
  minPressure = min(minPressure, pressure);
  maxPressure = max(maxPressure, pressure);
  minTemp = min(minTemp, temperature);
  maxTemp = max(maxTemp, temperature);
  minAltitude = min(minAltitude, altitude);
  maxAltitude = max(maxAltitude, altitude);

  Serial.print(millis());
  Serial.print("\t");
  Serial.print(temperature, 2);
  Serial.print("\t");
  Serial.print(pressure, 2);
  Serial.print("\t");
  Serial.println(altitude, 2);

  if (readingCount % 10 == 0) {
    Serial.println(F("\n--- Statistics ---"));
    Serial.print(F("Temp range: ")); Serial.print(minTemp, 2); Serial.print("–"); Serial.println(maxTemp, 2);
    Serial.print(F("Pressure range: ")); Serial.print(minPressure, 2); Serial.print("–"); Serial.println(maxPressure, 2);
    Serial.print(F("Altitude range: ")); Serial.print(minAltitude, 2); Serial.print("–"); Serial.println(maxAltitude, 2);
    Serial.println(F("------------------\n"));
    minPressure = 999999; maxPressure = 0;
    minTemp = 999999; maxTemp = -999999;
    minAltitude = 999999; maxAltitude = -999999;
  }

  delay(TEST_INTERVAL);
}
