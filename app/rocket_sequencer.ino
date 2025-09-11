/*
 * USLI Rocket Booster Sequencer - Main Application
 * Enhanced flight computer with full mission sequencing
 * Based on existing sensor framework with added state machine control
 * 
 * State Sequence:
 * SBIT (0-5):  Startup sequence
 * LBIT (6-9):  Launch sequence  
 * DBIT (10-15): Deploy sequence
 * ABIT (16):   Abort sequence
 */

#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>

// Include sequencer system
#include "sequencer.h"
#include "hardware_control.h"
#include "flight_detection.h"

// Include existing sensor framework
#include "baro.h"
#include "gps.h" 
#include "imu.h"
#include "temp.h"
#include "uSD.h"
#include "config.h"

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println(F("=== USLI Rocket Booster Sequencer v2.0 ==="));
  Serial.println(F("Initializing flight computer with mission sequencing..."));
  
  // Initialize hardware control systems
  initHardware();
  
  // Initialize sequencer
  initSequencer();
  
  // Initialize all sensors (reuse existing function with enhancements)
  initializeAllSensors();
  
  Serial.println(F("=== Sequencer Ready - Starting Mission ==="));
  blinkSequencerStatus();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Update GPS continuously (from existing code)
  updateGPS();
  
  // Check for emergency abort conditions
  checkEmergencyConditions();
  
  // Update sequencer state machine
  if (currentTime - sequencer.lastStateUpdate >= SEQUENCER_UPDATE_RATE_MS) {
    updateSequencer();
    sequencer.lastStateUpdate = currentTime;
  }
  
  // Collect telemetry at appropriate rate
  unsigned long telemRate = (sequencer.currentPhase == PHASE_LAUNCH || 
                            sequencer.currentPhase == PHASE_FLIGHT) ? 
                            FAST_TELEM_RATE_MS : NORMAL_TELEM_RATE_MS;
  
  static unsigned long lastTelemTime = 0;
  if (currentTime - lastTelemTime >= telemRate) {
    collectSequencerTelemetry();
    logSequencerData();
    lastTelemTime = currentTime;
  }
  
  // Update flight detection
  updateFlightDetection();
  
  // Status indication
  blinkSequencerStatus();
  
  delay(10); // Small delay for system stability
}

// Enhanced sensor initialization with sequencer integration
void initializeAllSensors() {
  Serial.println(F("\n=== Sensor Initialization ==="));
  
  // Try to initialize SD card (optional in test mode)
  Serial.println(F("Initializing SD card..."));
  sdCardAvailable = initSD();
  if (sdCardAvailable) {
    Serial.println(F(" SD card initialized successfully"));
    // Log initialization header with sequencer fields
    String header = "Timestamp,SeqState,SeqStateName,Phase,PhaseName,Temp_C,Pressure_Pa,Altitude_m,AltitudeAGL_m,VerticalVel_ms,Lat,Lon,GPS_Alt_m,Satellites,Accel_X,Accel_Y,Accel_Z,AccelMag,Gyro_X,Gyro_Y,Gyro_Z,Battery,Sensors,Payload";
    logData(header);
  } else {
    Serial.println(F("  SD card initialization failed - running in test mode"));
    Serial.println(F("   Check: CS pin configuration in config.h"));
  }
  
  // Initialize temperature sensor
  Serial.println(F("Initializing temperature sensor..."));
  initTempSensor();
  
  // Test temperature reading
  float tempTest = readTemperatureC();
  if (tempTest == -999.0) {
    Serial.println(F("  Temperature sensor not responding"));
    Serial.println(F("   Check: Data pin 8, VCC 5V, GND, 4.7kΩ pull-up resistor"));
  } else {
    Serial.print(F(" Temperature sensor working: "));
    Serial.print(tempTest);
    Serial.println(F("°C"));
  }
  
  // Initialize GPS
  Serial.println(F("Initializing GPS..."));
  initGPS();
  
  // Test GPS communication
  Serial.println(F("GPS: Waiting for satellite fix..."));
  Serial.println(F("   Note: GPS needs clear view of sky for satellite fix"));
  Serial.println(F("   Indoors, 0 satellites is normal"));
  
  // Initialize IMU when implemented
  Serial.println(F("IMU: Placeholder - implement when ICM-20948 code is ready"));
  // TODO: Uncomment when imu.cpp is implemented
  // initIMU();
  
  // Initialize barometer when implemented  
  Serial.println(F("Barometer: Placeholder - implement when BMP280 code is ready"));
  // TODO: Uncomment when baro.cpp is implemented
  // initBaro();
  
  Serial.println(F("=== Initialization Complete ===\n"));
}