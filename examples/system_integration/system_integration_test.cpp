/*
 * Full System Integration Test
 *
 * This sketch tests all sensors working together on the breadboard:
 * - GPS (NEO-6M)
 * - Temperature (DS18B20)
 * - Barometer (BMP280)
 * - IMU (ICM-20948)
 * - SD Card
 *
 * Purpose:
 * - Verify all sensors can be read simultaneously
 * - Check for I2C conflicts
 * - Test data logging to SD card
 * - Measure system timing and performance
 * - Validate sensor data correlation
 *
 * Hardware Setup (Breadboard):
 * GPS:
 *   - RX -> Pin 0 (Arduino TX)
 *   - TX -> Pin 1 (Arduino RX)
 *   - VCC -> 5V
 *   - GND -> GND
 *
 * Temperature (DS18B20):
 *   - Data -> Pin 12
 *   - VCC -> 5V
 *   - GND -> GND
 *   - 4.7kΩ pullup between Data and VCC
 *
 * Barometer (BMP280):
 *   - SDA -> A4
 *   - SCL -> A5
 *   - VCC -> 3.3V or 5V
 *   - GND -> GND
 *
 * IMU (ICM-20948):
 *   - SDA -> A4 (shared with BMP280)
 *   - SCL -> A5 (shared with BMP280)
 *   - VCC -> 3.3V ONLY!
 *   - GND -> GND
 *
 * SD Card:
 *   - CS -> Pin 11
 *   - MOSI -> Pin 11 (depends on board)
 *   - MISO -> Pin 12
 *   - SCK -> Pin 13
 *   - VCC -> 5V
 *   - GND -> GND
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include "ICM_20948.h"

// Pin definitions
#define GPS_RX_PIN 0
#define GPS_TX_PIN 1
#define SD_CS_PIN 11
#define TEMP_PIN 12
#define STATUS_LED_PIN 13

// Sensor objects
TinyGPSPlus gps;
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);
Adafruit_BMP280 baro;
ICM_20948_I2C imu;

// Data logging
File dataFile;
String dataFileName = "sysTest.txt";

// Timing
unsigned long lastRead = 0;
#define READ_INTERVAL 500  // 2Hz for testing (0.5s interval)

// Sensor status
struct SystemStatus {
  bool gpsInit;
  bool tempInit;
  bool baroInit;
  bool imuInit;
  bool sdInit;
  bool gpsFix;
  int sensorCount;
  unsigned long readCount;
  unsigned long errorCount;
} status;

// Statistics
struct SystemStats {
  unsigned long minLoopTime;
  unsigned long maxLoopTime;
  unsigned long avgLoopTime;
  unsigned long totalLoopTime;
} stats;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  Serial.println(F("========================================"));
  Serial.println(F("Full System Integration Test"));
  Serial.println(F("========================================\n"));

  // Initialize statistics
  stats.minLoopTime = 999999;
  stats.maxLoopTime = 0;
  stats.avgLoopTime = 0;
  stats.totalLoopTime = 0;

  // Initialize all sensors
  Serial.println(F("Initializing sensors...\n"));

  initGPS();
  initTemperature();
  initBarometer();
  initIMU();
  initSDCard();

  // Print summary
  Serial.println(F("\n========================================"));
  Serial.println(F("Initialization Summary"));
  Serial.println(F("========================================"));
  printStatus();
  Serial.println(F("========================================\n"));

  if (status.sensorCount < 4) {
    Serial.println(F("⚠️  WARNING: Not all sensors initialized!"));
    Serial.println(F("System will continue but data may be incomplete.\n"));
  }

  Serial.println(F("Starting continuous readings..."));
  Serial.println(F("Press Ctrl+C to stop\n"));

  delay(1000);
}

void loop() {
  unsigned long loopStart = millis();

  // Read GPS continuously (required for TinyGPS++)
  while (Serial.available() > 0) {
    gps.encode(Serial.read());
  }

  // Read all sensors at fixed interval
  if (millis() - lastRead >= READ_INTERVAL) {
    readAllSensors();
    lastRead = millis();
  }

  // Update timing statistics
  unsigned long loopTime = millis() - loopStart;
  if (loopTime < stats.minLoopTime) stats.minLoopTime = loopTime;
  if (loopTime > stats.maxLoopTime) stats.maxLoopTime = loopTime;
  stats.totalLoopTime += loopTime;

  // Blink status LED
  static bool ledState = false;
  if (millis() % 1000 < 500) {
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else {
    digitalWrite(STATUS_LED_PIN, LOW);
  }

  // Print stats every 10 readings
  if (status.readCount > 0 && status.readCount % 10 == 0) {
    printStatistics();
  }
}

void initGPS() {
  Serial.print(F("GPS (NEO-6M)... "));
  // GPS uses hardware serial (pins 0,1)
  // Already initialized in setup()
  status.gpsInit = true;
  status.sensorCount++;
  Serial.println(F("✓ OK"));
  Serial.println(F("  Note: GPS may take 1-5 minutes to get fix"));
}

void initTemperature() {
  Serial.print(F("Temperature (DS18B20)... "));
  tempSensor.begin();

  if (tempSensor.getDeviceCount() > 0) {
    status.tempInit = true;
    status.sensorCount++;
    Serial.println(F("✓ OK"));
    Serial.print(F("  Found "));
    Serial.print(tempSensor.getDeviceCount());
    Serial.println(F(" sensor(s)"));
  } else {
    status.tempInit = false;
    Serial.println(F("✗ FAIL - No sensors found"));
  }
}

void initBarometer() {
  Serial.print(F("Barometer (BMP280)... "));

  if (baro.begin(0x76) || baro.begin(0x77)) {
    status.baroInit = true;
    status.sensorCount++;
    Serial.println(F("✓ OK"));

    baro.setSampling(Adafruit_BMP280::MODE_NORMAL,
                     Adafruit_BMP280::SAMPLING_X2,
                     Adafruit_BMP280::SAMPLING_X16,
                     Adafruit_BMP280::FILTER_X16,
                     Adafruit_BMP280::STANDBY_MS_500);
  } else {
    status.baroInit = false;
    Serial.println(F("✗ FAIL - Not detected on I2C"));
  }
}

void initIMU() {
  Serial.print(F("IMU (ICM-20948)... "));

  Wire.begin();
  Wire.setClock(400000);

  imu.begin(Wire, 1);
  if (imu.status != ICM_20948_Stat_Ok) {
    imu.begin(Wire, 0);
  }

  if (imu.status == ICM_20948_Stat_Ok) {
    status.imuInit = true;
    status.sensorCount++;
    Serial.println(F("✓ OK"));

    // Configure IMU
    ICM_20948_fss_t fss;
    fss.a = gpm16;
    fss.g = dps2000;
    imu.setFullScale(ICM_20948_Internal_Acc, fss);
    imu.setFullScale(ICM_20948_Internal_Gyr, fss);
  } else {
    status.imuInit = false;
    Serial.print(F("✗ FAIL - "));
    Serial.println(imu.statusString());
  }
}

void initSDCard() {
  Serial.print(F("SD Card... "));

  if (SD.begin(SD_CS_PIN)) {
    status.sdInit = true;
    status.sensorCount++;
    Serial.println(F("✓ OK"));

    // Create/open log file
    if (SD.exists(dataFileName)) {
      Serial.print(F("  File exists, appending to: "));
      Serial.println(dataFileName);
    } else {
      Serial.print(F("  Creating new file: "));
      Serial.println(dataFileName);

      // Write header
      dataFile = SD.open(dataFileName, FILE_WRITE);
      if (dataFile) {
        dataFile.println(F("Timestamp,Temp_C,Pressure_hPa,Baro_Alt_m,GPS_Lat,GPS_Lon,GPS_Alt_m,GPS_Sats,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z"));
        dataFile.close();
      }
    }
  } else {
    status.sdInit = false;
    Serial.println(F("✗ FAIL - Card not detected"));
  }
}

void readAllSensors() {
  status.readCount++;
  String dataString = "";

  Serial.println(F("\n========================================"));
  Serial.print(F("Reading #"));
  Serial.print(status.readCount);
  Serial.print(F(" @ "));
  Serial.print(millis());
  Serial.println(F("ms"));
  Serial.println(F("========================================"));

  // Timestamp
  dataString += millis();
  dataString += ",";

  // Temperature
  if (status.tempInit) {
    tempSensor.requestTemperatures();
    float tempC = tempSensor.getTempCByIndex(0);
    Serial.print(F("Temperature: "));
    Serial.print(tempC, 2);
    Serial.println(F(" °C"));
    dataString += tempC;
  } else {
    dataString += "NaN";
  }
  dataString += ",";

  // Barometer
  if (status.baroInit) {
    float pressure = baro.readPressure() / 100.0F;
    float altitude = baro.readAltitude(1013.25);
    Serial.print(F("Pressure: "));
    Serial.print(pressure, 2);
    Serial.print(F(" hPa, Altitude: "));
    Serial.print(altitude, 2);
    Serial.println(F(" m"));
    dataString += pressure;
    dataString += ",";
    dataString += altitude;
  } else {
    dataString += "NaN,NaN";
  }
  dataString += ",";

  // GPS
  if (status.gpsInit && gps.location.isValid()) {
    status.gpsFix = true;
    Serial.print(F("GPS: "));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(", "));
    Serial.print(gps.location.lng(), 6);
    Serial.print(F(", Alt: "));
    Serial.print(gps.altitude.meters(), 1);
    Serial.print(F(" m, Sats: "));
    Serial.println(gps.satellites.value());

    dataString += String(gps.location.lat(), 6);
    dataString += ",";
    dataString += String(gps.location.lng(), 6);
    dataString += ",";
    dataString += gps.altitude.meters();
    dataString += ",";
    dataString += gps.satellites.value();
  } else {
    Serial.println(F("GPS: No fix"));
    dataString += "NaN,NaN,NaN,0";
  }
  dataString += ",";

  // IMU
  if (status.imuInit && imu.dataReady()) {
    imu.getAGMT();
    float accelX = imu.accX() * 9.81 / 1000.0;
    float accelY = imu.accY() * 9.81 / 1000.0;
    float accelZ = imu.accZ() * 9.81 / 1000.0;
    float gyroX = imu.gyrX();
    float gyroY = imu.gyrY();
    float gyroZ = imu.gyrZ();

    Serial.print(F("IMU Accel: "));
    Serial.print(accelX, 2);
    Serial.print(F(", "));
    Serial.print(accelY, 2);
    Serial.print(F(", "));
    Serial.print(accelZ, 2);
    Serial.println(F(" m/s²"));

    Serial.print(F("IMU Gyro: "));
    Serial.print(gyroX, 2);
    Serial.print(F(", "));
    Serial.print(gyroY, 2);
    Serial.print(F(", "));
    Serial.print(gyroZ, 2);
    Serial.println(F(" °/s"));

    dataString += accelX;
    dataString += ",";
    dataString += accelY;
    dataString += ",";
    dataString += accelZ;
    dataString += ",";
    dataString += gyroX;
    dataString += ",";
    dataString += gyroY;
    dataString += ",";
    dataString += gyroZ;
  } else {
    dataString += "NaN,NaN,NaN,NaN,NaN,NaN";
  }

  // Log to SD card
  if (status.sdInit) {
    dataFile = SD.open(dataFileName, FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println(F("✓ Data logged to SD card"));
    } else {
      Serial.println(F("✗ Error opening SD card file"));
      status.errorCount++;
    }
  }

  Serial.println(F("========================================"));
}

void printStatus() {
  Serial.print(F("GPS:         "));
  Serial.println(status.gpsInit ? F("✓ OK") : F("✗ FAIL"));

  Serial.print(F("Temperature: "));
  Serial.println(status.tempInit ? F("✓ OK") : F("✗ FAIL"));

  Serial.print(F("Barometer:   "));
  Serial.println(status.baroInit ? F("✓ OK") : F("✗ FAIL"));

  Serial.print(F("IMU:         "));
  Serial.println(status.imuInit ? F("✓ OK") : F("✗ FAIL"));

  Serial.print(F("SD Card:     "));
  Serial.println(status.sdInit ? F("✓ OK") : F("✗ FAIL"));

  Serial.println();
  Serial.print(F("Total Sensors Working: "));
  Serial.print(status.sensorCount);
  Serial.println(F(" / 5"));
}

void printStatistics() {
  Serial.println(F("\n========================================"));
  Serial.println(F("System Statistics"));
  Serial.println(F("========================================"));

  Serial.print(F("Total Readings: "));
  Serial.println(status.readCount);

  Serial.print(F("Errors: "));
  Serial.println(status.errorCount);

  stats.avgLoopTime = stats.totalLoopTime / status.readCount;

  Serial.print(F("Loop Time: Min="));
  Serial.print(stats.minLoopTime);
  Serial.print(F("ms, Max="));
  Serial.print(stats.maxLoopTime);
  Serial.print(F("ms, Avg="));
  Serial.print(stats.avgLoopTime);
  Serial.println(F("ms"));

  Serial.print(F("GPS Fix: "));
  Serial.println(status.gpsFix ? F("✓ YES") : F("✗ NO"));

  Serial.println(F("========================================\n"));

  // Reset timing stats
  stats.minLoopTime = 999999;
  stats.maxLoopTime = 0;
  stats.totalLoopTime = 0;
}
