#include "imu_icm20948.h"
#include "config.h"
#include <SPI.h>
#include "ICM_20948.h"  // SparkFun ICM-20948 library

// Create ICM20948 object
ICM_20948_SPI myICM;

bool initIMU() {
  SPI.begin();
  myICM.begin(IMU_CS_PIN, SPI);
  
  if (myICM.status != ICM_20948_Stat_Ok) return false;
  
  ICM_20948_fss_t myFSS;
  myFSS.a = gpm16;
  myFSS.g = dps2000;
  myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);
  
  return true;
}

bool readIMU(IMUData &data) {
  if (myICM.dataReady()) {
    myICM.getAGMT();
    
    data.accel_x = myICM.accX();
    data.accel_y = myICM.accY();
    data.accel_z = myICM.accZ();
    
    data.gyro_x = myICM.gyrX();
    data.gyro_y = myICM.gyrY();
    data.gyro_z = myICM.gyrZ();
    
    data.mag_x = 0;
    data.mag_y = 0;
    data.mag_z = 0;
    data.temperature = 0;
    
    data.dataValid = true;
    return true;
  }
  
  data.dataValid = false;
  return false;
}

void printIMUData(const IMUData &data) {
  if (!data.dataValid) {
    Serial.println(F("IMU: No valid data"));
    return;
  }
  
  Serial.print(F("IMU - Accel: "));
  Serial.print(data.accel_x, 2); Serial.print(F(", "));
  Serial.print(data.accel_y, 2); Serial.print(F(", "));
  Serial.print(data.accel_z, 2); Serial.print(F(" m/s² | "));
  
  Serial.print(F("Gyro: "));
  Serial.print(data.gyro_x, 2); Serial.print(F(", "));
  Serial.print(data.gyro_y, 2); Serial.print(F(", "));
  Serial.print(data.gyro_z, 2); Serial.print(F(" rad/s | "));
  
  Serial.print(F("Mag: "));
  Serial.print(data.mag_x, 1); Serial.print(F(", "));
  Serial.print(data.mag_y, 1); Serial.print(F(", "));
  Serial.print(data.mag_z, 1); Serial.print(F(" µT | "));
  
  Serial.print(F("Temp: "));
  Serial.print(data.temperature, 1); Serial.println(F("°C"));
}

bool isIMUConnected() {
  return (myICM.status == ICM_20948_Stat_Ok);
}

// Conversion functions
float convertAccel(int16_t raw) {
  // Convert from raw to m/s² based on ±16g range
  return (raw / 2048.0) * 9.81; // 2048 LSB/g for ±16g range
}

float convertGyro(int16_t raw) {
  // Convert from raw to rad/s based on ±2000 dps range
  return (raw / 16.4) * (PI / 180.0); // 16.4 LSB/dps for ±2000 dps range
}

float convertMag(int16_t raw) {
  // Convert from raw to µT
  return raw * 0.15; // 0.15 µT/LSB for magnetometer
}

float convertTemp(int16_t raw) {
  // Convert from raw to °C
  return ((raw - 21) / 333.87) + 21; // ICM-20948 temperature formula
}