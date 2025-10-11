#include <Arduino.h>
#include <SPI.h>
#include "ICM_20948.h"  // SparkFun ICM-20948 library

#define IMU_CS   3
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK  13

#define TEST_INTERVAL 100
#define SERIAL_BAUD 115200

ICM_20948_SPI myICM;  // SPI object

int readingCount = 0;
float accelMagnitude = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;

void calibrateGyro();

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) delay(10);

  Serial.println(F("========================================"));
  Serial.println(F("ICM-20948 9-Axis IMU Test (SPI Mode)"));
  Serial.println(F("========================================\n"));

  Serial.println(F("Initializing SPI bus..."));
  SPI.begin();  // On Arduino Nano, SPI pins are fixed (MOSI=11, MISO=12, SCK=13)
  pinMode(IMU_CS, OUTPUT);
  digitalWrite(IMU_CS, HIGH);

  Serial.println(F("Initializing ICM-20948 via SPI..."));
  myICM.begin(IMU_CS, SPI);

  if (myICM.status != ICM_20948_Stat_Ok) {
    Serial.print(F("✗ Initialization failed! Status: "));
    Serial.println(myICM.statusString());
    while (1) delay(1000);
  }

  Serial.println(F("✓ ICM-20948 initialized successfully!"));

  // Configure accelerometer and gyroscope
  ICM_20948_fss_t accelFS; accelFS.a = gpm16;
  ICM_20948_fss_t gyroFS;  gyroFS.g = dps2000;
  myICM.setFullScale(ICM_20948_Internal_Acc, accelFS);
  myICM.setFullScale(ICM_20948_Internal_Gyr, gyroFS);

  // Set sample rate (~100 Hz)
  ICM_20948_smplrt_t rate; rate.a = 10; rate.g = 10;
  myICM.setSampleRate(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr, rate);

  Serial.println(F("Configuration complete:"));
  Serial.println(F("  - Mode: SPI"));
  Serial.println(F("  - Accel Range: ±16g"));
  Serial.println(F("  - Gyro Range: ±2000°/s"));
  Serial.println(F("  - Sample Rate: ~100Hz"));
  Serial.println(F("\n========================================"));
  Serial.println(F("CALIBRATION: Keep IMU stationary"));
  Serial.println(F("========================================"));
  calibrateGyro();
  Serial.println(F("✓ Calibration complete\n"));

  Serial.println(F("Starting continuous readings..."));
  Serial.println(F("Legend: Accel(m/s²) | Gyro(°/s) | Mag(µT) | Temp(°C)"));
  Serial.println(F("========================================"));
}

void loop() {
  if (myICM.dataReady()) {
    myICM.getAGMT();
    readingCount++;

    // Convert accelerometer data (m/s²)
    float accelX = myICM.accX() * 9.81 / 1000.0;
    float accelY = myICM.accY() * 9.81 / 1000.0;
    float accelZ = myICM.accZ() * 9.81 / 1000.0;
    accelMagnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    // Gyroscope data (°/s) minus offset
    float gyroX = myICM.gyrX() - gyroOffsetX;
    float gyroY = myICM.gyrY() - gyroOffsetY;
    float gyroZ = myICM.gyrZ() - gyroOffsetZ;

    // Magnetometer data (µT)
    float magX = myICM.magX();
    float magY = myICM.magY();
    float magZ = myICM.magZ();

    // Temperature (°C)
    float temp = myICM.temp();

    Serial.print(F("Reading #"));
    Serial.print(readingCount);
    Serial.print(F(" @ "));
    Serial.print(millis());
    Serial.println(F("ms"));
    Serial.print(F("  Accel: X=")); Serial.print(accelX, 2);
    Serial.print(F(" Y=")); Serial.print(accelY, 2);
    Serial.print(F(" Z=")); Serial.print(accelZ, 2);
    Serial.print(F(" |Mag|=")); Serial.println(accelMagnitude, 2);

    Serial.print(F("  Gyro:  X=")); Serial.print(gyroX, 2);
    Serial.print(F(" Y=")); Serial.print(gyroY, 2);
    Serial.print(F(" Z=")); Serial.println(gyroZ, 2);

    Serial.print(F("  Mag:   X=")); Serial.print(magX, 2);
    Serial.print(F(" Y=")); Serial.print(magY, 2);
    Serial.print(F(" Z=")); Serial.println(magZ, 2);

    Serial.print(F("  Temp:  "));
    Serial.print(temp, 1);
    Serial.println(F("°C\n"));

  } else {
    Serial.println(F("⚠ Waiting for data..."));
  }

  delay(TEST_INTERVAL);
}

void calibrateGyro() {
  const int numSamples = 100;
  float sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < numSamples; i++) {
    if (myICM.dataReady()) {
      myICM.getAGMT();
      sumX += myICM.gyrX();
      sumY += myICM.gyrY();
      sumZ += myICM.gyrZ();
    }
    delay(10);
    if (i % 10 == 0) Serial.print(F("."));
  }

  Serial.println();
  gyroOffsetX = sumX / numSamples;
  gyroOffsetY = sumY / numSamples;
  gyroOffsetZ = sumZ / numSamples;

  Serial.print(F("  Gyro Offsets: X="));
  Serial.print(gyroOffsetX, 2);
  Serial.print(F(" Y="));
  Serial.print(gyroOffsetY, 2);
  Serial.print(F(" Z="));
  Serial.println(gyroOffsetZ, 2);
}
