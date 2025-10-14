#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#define BMP_CS   3
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK  13

#define TEST_INTERVAL 1000 // 1 second, if you do 0.5 seconds it might be buggy
#define SEA_LEVEL_PRESSURE 1013.25  // hPa
#define N 5                         // median filter window size

Adafruit_BMP280 bmp(BMP_CS, SPI_MOSI, SPI_MISO, SPI_SCK);

// Statistics tracking
float minPressure = 999999, maxPressure = 0;
float minTemp = 999999, maxTemp = -999999;
float minAltitude = 999999, maxAltitude = -999999;
int readingCount = 0;

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
  float pressure = bmp.readPressure() / 100.0F; // Pa → hPa

  // Validate reading
  if (!validReading(temperature, pressure)) {
    delay(10);
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0F;
    if (!validReading(temperature, pressure)) {
      Serial.println(F("✗ Invalid sensor read!"));
      delay(TEST_INTERVAL);
      return;
    }
  }

  float altitude = calculateAltitude(pressure, SEA_LEVEL_PRESSURE);
  altitude = filterAltitude(altitude);

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
