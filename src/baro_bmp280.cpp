#include "baro_bmp280.h"
#include "config.h"
#include "sequencer.h"

// Create BMP280 object
Adafruit_BMP280 bmp(BARO_CS_PIN); // Hardware SPI

// Sea level pressure for altitude calculation (adjust for location)
static float seaLevelPressure = 1013.25; // hPa

bool initBaro() {
  Serial.println(F("Initializing BMP280 Barometer..."));
  
  if (!bmp.begin()) {
    Serial.println(F(" BMP280 sensor not found"));
    Serial.println(F("   Check: SPI wiring, CS pin configuration"));
    return false;
  }
  
  // Configure BMP280 settings
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time
  
  Serial.println(F(" BMP280 initialized successfully"));
  
  // Read initial pressure for sea level calibration
  delay(100); // Let sensor stabilize
  float initialPressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  
  Serial.print(F("Initial pressure: "));
  Serial.print(initialPressure);
  Serial.println(F(" hPa"));
  
  return true;
}

bool readBaro(BaroData &data) {
  // Read pressure and temperature
  float pressure = bmp.readPressure();
  float temperature = bmp.readTemperature();
  
  // Check for valid readings
  if (isnan(pressure) || isnan(temperature)) {
    data.dataValid = false;
    return false;
  }
  
  // Convert pressure from Pa to hPa
  data.pressure = pressure / 100.0F;
  data.temperature = temperature;
  
  // Calculate altitude
  data.altitude = calculateAltitude(data.pressure, seaLevelPressure);
  
  data.dataValid = true;
  
  // Update sequencer data
  sequencerData.pressure = data.pressure;
  sequencerData.altitude = data.altitude;
  
  return true;
}

void printBaroData(const BaroData &data) {
  if (!data.dataValid) {
    Serial.println(F("Baro: No valid data"));
    return;
  }
  
  Serial.print(F("Baro - Pressure: "));
  Serial.print(data.pressure, 2);
  Serial.print(F(" hPa | Temp: "));
  Serial.print(data.temperature, 1);
  Serial.print(F("°C | Alt: "));
  Serial.print(data.altitude, 1);
  Serial.println(F(" m"));
}

bool isBaroConnected() {
  // Try to read the sensor ID
  return bmp.begin();
}

float calculateAltitude(float pressure, float seaLevelPressure) {
  // Barometric formula for altitude calculation
  return 44330.0 * (1.0 - pow(pressure / seaLevelPressure, 0.1903));
}

// Function to set sea level pressure for altitude calibration
void setSeaLevelPressure(float pressure) {
  seaLevelPressure = pressure;
  Serial.print(F("Sea level pressure set to: "));
  Serial.print(seaLevelPressure);
  Serial.println(F(" hPa"));
}

// Function to calibrate altitude to current position
void calibrateAltitude(float currentAltitude) {
  if (isBaroConnected()) {
    float currentPressure = bmp.readPressure() / 100.0F; // Pa to hPa
    
    // Calculate what sea level pressure should be for this altitude
    seaLevelPressure = currentPressure / pow(1.0 - (currentAltitude / 44330.0), 5.255);
    
    Serial.print(F("Altitude calibrated to: "));
    Serial.print(currentAltitude);
    Serial.print(F(" m, Sea level pressure: "));
    Serial.print(seaLevelPressure);
    Serial.println(F(" hPa"));
  }
}