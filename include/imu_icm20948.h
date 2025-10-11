#ifndef IMU_ICM20948_H
#define IMU_ICM20948_H

#include <Arduino.h>
// #include "ICM_20948.h"

// IMU data structure
struct IMUData {
  float accel_x, accel_y, accel_z;  // m/s²
  float gyro_x, gyro_y, gyro_z;     // rad/s
  float mag_x, mag_y, mag_z;        // µT
  float temperature;                // °C
  bool dataValid;
};

// Function prototypes
bool initIMU();
bool readIMU(IMUData &data);
void printIMUData(const IMUData &data);
bool isIMUConnected();

// Conversion functions
float convertAccel(int16_t raw);
float convertGyro(int16_t raw);
float convertMag(int16_t raw);
float convertTemp(int16_t raw);

#endif // IMU_ICM20948_H