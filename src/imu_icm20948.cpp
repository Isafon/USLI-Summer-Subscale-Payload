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

// Unused functions removed to save memory