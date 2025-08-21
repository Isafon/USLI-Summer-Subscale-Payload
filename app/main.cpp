#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>

// Include configuration and sensor headers
#include "config.h"
#include "../include/baro.h"
#include "../include/gps.h"
#include "../include/imu.h"
#include "../include/temp.h"
#include "../include/uSD.h"

// Flight state tracking
enum FlightState {
  PREFLIGHT,
  FLIGHT,
  POSTFLIGHT
};

FlightState currentState = PREFLIGHT;
unsigned long lastSampleTime = 0;
unsigned long flightStartTime = 0;
bool flightStarted = false;

// Telemetry data structure
struct TelemetryData {
  unsigned long timestamp;
  float temperature;
  float pressure;
  float altitude;
  float gps_lat;
  float gps_lon;
  float gps_alt;
  int gps_satellites;
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  FlightState state;
};

TelemetryData currentData;

// Function prototypes
void initializeAllSensors();
void collectTelemetryData();
void logTelemetryData();
void determineFlightState();
String formatTelemetryString();
void printTelemetryToSerial();

void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println(F("=== USLI Summer Subscale Payload ==="));
  Serial.println(F("Initializing sensors..."));
  
  // Initialize all sensors
  initializeAllSensors();
  
  Serial.println(F("All sensors initialized successfully!"));
  Serial.println(F("Waiting for flight conditions..."));
  
  // Initialize timing
  lastSampleTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Determine current flight state
  determineFlightState();
  
  // Collect data based on flight state
  unsigned long sampleInterval = (currentState == FLIGHT) ? 
    FLIGHT_SAMPLE_RATE_MS : 
    (currentState == PREFLIGHT ? PREFLIGHT_SAMPLE_RATE_MS : POSTFLIGHT_SAMPLE_RATE_MS);
  
  if (currentTime - lastSampleTime >= sampleInterval) {
    collectTelemetryData();
    logTelemetryData();
    printTelemetryToSerial();
    lastSampleTime = currentTime;
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}

void initializeAllSensors() {
  // Initialize SD card first (critical for data logging)
  if (!initSD()) {
    Serial.println(F("ERROR: SD card initialization failed!"));
    while (1) {
      delay(1000);
      Serial.println(F("SD card required for operation"));
    }
  }
  
  // Initialize temperature sensor
  initTempSensor();
  
  // Initialize GPS
  // Note: GPS setup is handled in gps.cpp, but we need to ensure it's ready
  Serial.println(F("GPS: Waiting for satellite fix..."));
  
  // Initialize IMU (placeholder - implement when imu.h/cpp are ready)
  // initIMU();
  
  // Initialize barometer (placeholder - implement when baro.h/cpp are ready)
  // initBaro();
  
  // Log initialization header
  String header = "Timestamp,Temp_C,Pressure_Pa,Altitude_m,Lat,Lon,GPS_Alt_m,Satellites,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,FlightState";
  logData(header);
}

void collectTelemetryData() {
  currentData.timestamp = millis();
  currentData.state = currentState;
  
  // Read temperature
  currentData.temperature = readTemperatureC();
  
  // Read GPS data (from gps.cpp objects)
  if (gps.location.isValid()) {
    currentData.gps_lat = gps.location.lat();
    currentData.gps_lon = gps.location.lng();
  } else {
    currentData.gps_lat = 0.0;
    currentData.gps_lon = 0.0;
  }
  
  if (gps.altitude.isValid()) {
    currentData.gps_alt = gps.altitude.meters();
  } else {
    currentData.gps_alt = 0.0;
  }
  
  currentData.gps_satellites = gps.satellites.value();
  
  // Read barometer data (placeholder)
  currentData.pressure = 0.0;
  currentData.altitude = 0.0;
  
  // Read IMU data (placeholder)
  currentData.accel_x = 0.0;
  currentData.accel_y = 0.0;
  currentData.accel_z = 0.0;
  currentData.gyro_x = 0.0;
  currentData.gyro_y = 0.0;
  currentData.gyro_z = 0.0;
}

void determineFlightState() {
  // Simple flight detection based on altitude change
  // This is a basic implementation - you may want to enhance this logic
  
  static float lastAltitude = 0.0;
  static unsigned long lastAltitudeCheck = 0;
  
  // Check altitude periodically
  if (millis() - lastAltitudeCheck >= ALTITUDE_CHECK_INTERVAL_MS) {
    float currentAltitude = currentData.altitude;
    
    if (!flightStarted && currentAltitude > lastAltitude + ALTITUDE_RISE_THRESHOLD_M) {
      // Altitude increased significantly - likely flight start
      flightStarted = true;
      flightStartTime = millis();
      currentState = FLIGHT;
      Serial.println(F("FLIGHT DETECTED!"));
    } else if (flightStarted && currentAltitude < lastAltitude - ALTITUDE_FALL_THRESHOLD_M) {
      // Altitude decreased significantly - likely landing
      currentState = POSTFLIGHT;
      Serial.println(F("LANDING DETECTED"));
    }
    
    lastAltitude = currentAltitude;
    lastAltitudeCheck = millis();
  }
}

void logTelemetryData() {
  String dataString = formatTelemetryString();
  if (!logData(dataString)) {
    Serial.println(F("ERROR: Failed to log data to SD card"));
  }
}

String formatTelemetryString() {
  String dataString = "";
  dataString += currentData.timestamp;
  dataString += ",";
  dataString += currentData.temperature;
  dataString += ",";
  dataString += currentData.pressure;
  dataString += ",";
  dataString += currentData.altitude;
  dataString += ",";
  dataString += String(currentData.gps_lat, 6);
  dataString += ",";
  dataString += String(currentData.gps_lon, 6);
  dataString += ",";
  dataString += currentData.gps_alt;
  dataString += ",";
  dataString += currentData.gps_satellites;
  dataString += ",";
  dataString += currentData.accel_x;
  dataString += ",";
  dataString += currentData.accel_y;
  dataString += ",";
  dataString += currentData.accel_z;
  dataString += ",";
  dataString += currentData.gyro_x;
  dataString += ",";
  dataString += currentData.gyro_y;
  dataString += ",";
  dataString += currentData.gyro_z;
  dataString += ",";
  
  switch (currentData.state) {
    case PREFLIGHT:
      dataString += "PREFLIGHT";
      break;
    case FLIGHT:
      dataString += "FLIGHT";
      break;
    case POSTFLIGHT:
      dataString += "POSTFLIGHT";
      break;
  }
  
  return dataString;
}

void printTelemetryToSerial() {
  // Print a summary to serial for debugging/monitoring
  Serial.print(F("T: "));
  Serial.print(currentData.temperature);
  Serial.print(F("°C, Alt: "));
  Serial.print(currentData.altitude);
  Serial.print(F("m, GPS: "));
  Serial.print(currentData.gps_satellites);
  Serial.print(F(" sats, State: "));
  
  switch (currentData.state) {
    case PREFLIGHT:
      Serial.print(F("PREFLIGHT"));
      break;
    case FLIGHT:
      Serial.print(F("FLIGHT"));
      break;
    case POSTFLIGHT:
      Serial.print(F("POSTFLIGHT"));
      break;
  }
  Serial.println();
}
