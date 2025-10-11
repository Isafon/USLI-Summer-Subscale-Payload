/*
 * ICM-20948 9-Axis IMU Test
 *
 * This sketch tests the ICM-20948 IMU to verify:
 * - I2C communication is working
 * - Sensor initializes correctly
 * - Accelerometer readings are valid
 * - Gyroscope readings are valid
 * - Magnetometer readings are valid
 * - Temperature readings are valid
 *
 * Hardware Connections:
 * - VCC -> 3.3V (ICM-20948 is 3.3V ONLY!)
 * - GND -> GND
 * - SDA -> A4 (Arduino Nano)
 * - SCL -> A5 (Arduino Nano)
 *
 * Expected Output (at rest):
 * - Accel X/Y: ~0 m/s²
 * - Accel Z: ~9.81 m/s² (gravity)
 * - Gyro X/Y/Z: ~0 deg/s (small drift is normal)
 * - Mag: Varies based on orientation
 */

#include <Arduino.h>
#include <Wire.h>
#include "ICM_20948.h"

ICM_20948_I2C myICM;

// Test parameters
#define TEST_INTERVAL 100  // Read sensor every 100ms (10Hz)
#define SERIAL_BAUD 115200

// Statistics tracking
int readingCount = 0;
float accelMagnitude = 0;
bool calibrationComplete = false;

// Calibration offsets
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) delay(10);

  Serial.println(F("========================================"));
  Serial.println(F("ICM-20948 9-Axis IMU Test"));
  Serial.println(F("========================================"));
  Serial.println();

  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C

  // Test I2C connection
  Serial.println(F("Testing I2C bus..."));
  Wire.beginTransmission(0x69); // ICM-20948 default address
  byte error = Wire.endTransmission();

  if (error == 0) {
    Serial.println(F("✓ ICM-20948 detected at address 0x69"));
  } else {
    Wire.beginTransmission(0x68); // Alternative address
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.println(F("✓ ICM-20948 detected at address 0x68"));
    } else {
      Serial.println(F("✗ ICM-20948 NOT detected on I2C bus!"));
      Serial.println(F("  Check wiring:"));
      Serial.println(F("    - SDA -> A4"));
      Serial.println(F("    - SCL -> A5"));
      Serial.println(F("    - VCC -> 3.3V (NOT 5V!)"));
      Serial.println(F("    - GND -> GND"));
      Serial.println(F("  Note: ICM-20948 requires 3.3V!"));
      while (1) delay(1000);
    }
  }

  // Initialize sensor
  Serial.println(F("\nInitializing ICM-20948..."));
  bool initialized = false;

  myICM.begin(Wire, 1); // AD0_VAL = 1 (address 0x69)

  if (myICM.status != ICM_20948_Stat_Ok) {
    myICM.begin(Wire, 0); // Try AD0_VAL = 0 (address 0x68)
  }

  if (myICM.status == ICM_20948_Stat_Ok) {
    Serial.println(F("✓ ICM-20948 initialized successfully!"));
    initialized = true;
  } else {
    Serial.print(F("✗ Failed to initialize ICM-20948! Status: "));
    Serial.println(myICM.statusString());
    while (1) delay(1000);
  }

  // Configure sensor
  Serial.println(F("\nConfiguring sensor..."));

  // Configure accelerometer (±16g range)
  ICM_20948_fss_t accelFS;
  accelFS.a = gpm16; // ±16g
  myICM.setFullScale(ICM_20948_Internal_Acc, accelFS);

  // Configure gyroscope (±2000 dps range)
  ICM_20948_fss_t gyroFS;
  gyroFS.g = dps2000; // ±2000 deg/s
  myICM.setFullScale(ICM_20948_Internal_Gyr, gyroFS);

  // Set sample rate divider for 100Hz (1125/(1+10) = 102Hz)
  ICM_20948_smplrt_t sampleRate;
  sampleRate.a = 10;
  sampleRate.g = 10;
  myICM.setSampleRate(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr, sampleRate);

  Serial.println(F("  - Accelerometer: ±16g range"));
  Serial.println(F("  - Gyroscope: ±2000 deg/s range"));
  Serial.println(F("  - Sample Rate: ~100Hz"));

  // Calibrate gyroscope
  Serial.println(F("\n========================================"));
  Serial.println(F("CALIBRATION: Keep IMU stationary!"));
  Serial.println(F("Calibrating gyroscope..."));
  calibrateGyro();
  Serial.println(F("✓ Calibration complete!"));
  Serial.println(F("========================================\n"));

  Serial.println(F("Starting continuous readings..."));
  Serial.println(F("Legend: Accel(m/s²) | Gyro(°/s) | Mag(µT) | Temp(°C)"));
  Serial.println(F("========================================"));

  delay(500);
}

void loop() {
  if (myICM.dataReady()) {
    myICM.getAGMT(); // Read all sensors
    readingCount++;

    // Get accelerometer data (convert to m/s²)
    float accelX = myICM.accX() * 9.81 / 1000.0;
    float accelY = myICM.accY() * 9.81 / 1000.0;
    float accelZ = myICM.accZ() * 9.81 / 1000.0;

    // Calculate magnitude
    accelMagnitude = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);

    // Get gyroscope data (already in deg/s, apply calibration)
    float gyroX = myICM.gyrX() - gyroOffsetX;
    float gyroY = myICM.gyrY() - gyroOffsetY;
    float gyroZ = myICM.gyrZ() - gyroOffsetZ;

    // Get magnetometer data (µT)
    float magX = myICM.magX();
    float magY = myICM.magY();
    float magZ = myICM.magZ();

    // Get temperature
    float temp = myICM.temp();

    // Print formatted data
    Serial.print(F("Reading #"));
    Serial.print(readingCount);
    Serial.print(F(" @ "));
    Serial.print(millis());
    Serial.println(F("ms"));

    Serial.print(F("  Accel: X="));
    Serial.print(accelX, 2);
    Serial.print(F(" Y="));
    Serial.print(accelY, 2);
    Serial.print(F(" Z="));
    Serial.print(accelZ, 2);
    Serial.print(F(" |Mag|="));
    Serial.println(accelMagnitude, 2);

    Serial.print(F("  Gyro:  X="));
    Serial.print(gyroX, 2);
    Serial.print(F(" Y="));
    Serial.print(gyroY, 2);
    Serial.print(F(" Z="));
    Serial.println(gyroZ, 2);

    Serial.print(F("  Mag:   X="));
    Serial.print(magX, 2);
    Serial.print(F(" Y="));
    Serial.print(magY, 2);
    Serial.print(F(" Z="));
    Serial.println(magZ, 2);

    Serial.print(F("  Temp:  "));
    Serial.print(temp, 1);
    Serial.println(F("°C"));
    Serial.println();

    // Run validation after 10 readings
    if (readingCount == 10) {
      Serial.println(F("========================================"));
      Serial.println(F("Running validation checks..."));
      Serial.println(F("========================================"));

      // Check 1: Gravity check (should be ~9.81 m/s²)
      if (accelMagnitude > 8.5 && accelMagnitude < 11.0) {
        Serial.println(F("✓ Accelerometer gravity check PASSED"));
        Serial.print(F("  Measured: "));
        Serial.print(accelMagnitude, 2);
        Serial.println(F(" m/s² (expected ~9.81)"));
      } else {
        Serial.println(F("⚠ WARNING: Accelerometer magnitude unexpected!"));
        Serial.print(F("  Measured: "));
        Serial.print(accelMagnitude, 2);
        Serial.println(F(" m/s² (expected ~9.81)"));
      }

      // Check 2: Gyro drift (should be near zero)
      float gyroDrift = sqrt(gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ);
      if (gyroDrift < 5.0) {
        Serial.println(F("✓ Gyroscope drift check PASSED"));
        Serial.print(F("  Drift magnitude: "));
        Serial.print(gyroDrift, 2);
        Serial.println(F(" °/s (expected <5)"));
      } else {
        Serial.println(F("⚠ WARNING: Gyroscope drift high!"));
        Serial.print(F("  Drift magnitude: "));
        Serial.print(gyroDrift, 2);
        Serial.println(F(" °/s"));
        Serial.println(F("  Try recalibrating (reset Arduino while stationary)"));
      }

      // Check 3: Temperature range
      if (temp > 15 && temp < 40) {
        Serial.println(F("✓ Temperature check PASSED"));
        Serial.print(F("  Temperature: "));
        Serial.print(temp, 1);
        Serial.println(F("°C"));
      } else {
        Serial.println(F("⚠ Temperature unusual"));
      }

      // Check 4: Magnetometer sanity
      float magMagnitude = sqrt(magX*magX + magY*magY + magZ*magZ);
      if (magMagnitude > 20 && magMagnitude < 200) {
        Serial.println(F("✓ Magnetometer check PASSED"));
        Serial.print(F("  Magnetic field: "));
        Serial.print(magMagnitude, 1);
        Serial.println(F(" µT (typical: 25-65)"));
      } else {
        Serial.println(F("⚠ WARNING: Magnetometer reading unusual!"));
        Serial.print(F("  Magnetic field: "));
        Serial.print(magMagnitude, 1);
        Serial.println(F(" µT"));
      }

      Serial.println(F("========================================\n"));
    }

    // Motion detection examples
    if (readingCount > 10 && readingCount % 10 == 0) {
      Serial.println(F("--- Motion Detection ---"));

      // High acceleration detection (launch)
      if (accelMagnitude > 20.0) {
        Serial.print(F("⚡ HIGH ACCELERATION DETECTED! "));
        Serial.print(accelMagnitude / 9.81, 1);
        Serial.println(F("G"));
      }

      // Rotation detection
      float rotationRate = sqrt(gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ);
      if (rotationRate > 50.0) {
        Serial.print(F("🔄 ROTATION DETECTED! "));
        Serial.print(rotationRate, 1);
        Serial.println(F(" °/s"));
      }

      // Freefall detection
      if (accelMagnitude < 2.0) {
        Serial.println(F("🪂 FREEFALL/LOW-G DETECTED!"));
      }

      Serial.println(F("------------------------\n"));
    }

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

    if (i % 10 == 0) {
      Serial.print(F("."));
    }
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
