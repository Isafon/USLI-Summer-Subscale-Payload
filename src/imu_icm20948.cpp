#include "imu_icm20948.h"
#include "config.h"
#include "sequencer.h"

// Create ICM20948 object
ICM_20948_SPI myICM;

bool initIMU() {
  Serial.println(F("Initializing ICM-20948 IMU..."));
  
  // Initialize SPI
  SPI.begin();
  
  // Initialize the ICM-20948
  bool initialized = false;
  myICM.begin(IMU_CS_PIN, SPI);
  
  Serial.print(F("Initialization of the sensor returned: "));
  Serial.println(myICM.statusString());
  
  if (myICM.status != ICM_20948_Stat_Ok) {
    Serial.println(F(" ICM-20948 initialization failed"));
    Serial.println(F("   Check: SPI wiring, CS pin configuration"));
    return false;
  }
  
  // Configure the ICM-20948
  Serial.println(F("Configuring ICM-20948..."));
  
  // Set full scale ranges and digital low pass filter
  ICM_20948_Status_e retval = ICM_20948_Stat_Ok;
  
  // Set gyro full scale to ±2000 dps
  ICM_20948_fss_t myFSS;
  myFSS.a = gpm16;  // Accelerometer ±16g
  myFSS.g = dps2000; // Gyroscope ±2000 dps
  
  retval = myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);
  if (retval != ICM_20948_Stat_Ok) {
    Serial.println(F(" Failed to set full scale ranges"));
    return false;
  }
  
  // Set digital low pass filter
  ICM_20948_dlpcfg_t myDLPcfg;
  myDLPcfg.a = acc_d473bw_n499bw;  // Accelerometer DLPF
  myDLPcfg.g = gyr_d361bw4_n376bw5; // Gyroscope DLPF
  
  retval = myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg);
  if (retval != ICM_20948_Stat_Ok) {
    Serial.println(F("  Failed to set DLPF configuration"));
    return false;
  }
  
  // Enable DLPF
  retval = myICM.enableDLPF(ICM_20948_Internal_Acc, true);
  retval = myICM.enableDLPF(ICM_20948_Internal_Gyr, true);
  if (retval != ICM_20948_Stat_Ok) {
    Serial.println(F("  Failed to enable DLPF"));
    return false;
  }
  
  // Set sample rates for 2Hz data collection (sensor still samples faster internally)
  // Note: ICM-20948 internal sample rate is higher than our 2Hz polling rate
  // We configure for reasonable internal sampling and read at 2Hz in main loop
  ICM_20948_smplrt_t mySmplrt;
  mySmplrt.g = 19; // ODR = 1100/(1+19) = 55Hz for gyro (internal rate)
  mySmplrt.a = 19; // ODR = 1125/(1+19) = 56.25Hz for accel (internal rate)
  
  retval = myICM.setSampleRate((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), mySmplrt);
  if (retval != ICM_20948_Stat_Ok) {
    Serial.println(F("  Failed to set sample rates"));
    return false;
  }
  
  Serial.println(F(" ICM-20948 initialized successfully"));
  return true;
}

bool readIMU(IMUData &data) {
  if (myICM.dataReady()) {
    myICM.getAGMT(); // Get accelerometer, gyroscope, magnetometer, and temperature data
    
    // Convert raw data to engineering units
    data.accel_x = convertAccel(myICM.agmt.acc.axes.x);
    data.accel_y = convertAccel(myICM.agmt.acc.axes.y);
    data.accel_z = convertAccel(myICM.agmt.acc.axes.z);
    
    data.gyro_x = convertGyro(myICM.agmt.gyr.axes.x);
    data.gyro_y = convertGyro(myICM.agmt.gyr.axes.y);
    data.gyro_z = convertGyro(myICM.agmt.gyr.axes.z);
    
    data.mag_x = convertMag(myICM.agmt.mag.axes.x);
    data.mag_y = convertMag(myICM.agmt.mag.axes.y);
    data.mag_z = convertMag(myICM.agmt.mag.axes.z);
    
    data.temperature = convertTemp(myICM.agmt.tmp.val);
    
    data.dataValid = true;
    
    // Update sequencer data
    sequencerData.accel_x = data.accel_x / 9.81; // Convert to g's
    sequencerData.accel_y = data.accel_y / 9.81;
    sequencerData.accel_z = data.accel_z / 9.81;
    sequencerData.gyro_x = data.gyro_x;
    sequencerData.gyro_y = data.gyro_y;
    sequencerData.gyro_z = data.gyro_z;
    
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