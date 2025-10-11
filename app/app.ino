/*
 * USLI Summer Subscale Payload - Main Sketch
 * Rocket Telemetry System for Arduino Nano v4
 */

 #include <Arduino.h>
 #include <SD.h>
 
 // Include our sensor headers
 #include "baro.h"
 #include "imu.h"
 #include "uSD.h"
 
 // Include configuration
 #include "config.h"
 
 // LED pin for status indication
 #define STATUS_LED_PIN 13
 
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
 bool sdCardAvailable = false;
 
 // Telemetry data structure
 struct TelemetryData {
   unsigned long timestamp;
   float pressure;
   float altitude;
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
 void blinkLED(int times);
 
 void setup() {
   // Initialize serial communication
   Serial.begin(SERIAL_BAUD_RATE);
   Serial.println(F("=== USLI Summer Subscale Payload ==="));
   Serial.println(F("Initializing sensors..."));
   
   // Initialize status LED
   pinMode(STATUS_LED_PIN, OUTPUT);
   blinkLED(3); // 3 blinks = startup
   
   // Initialize all sensors
   initializeAllSensors();
   
   Serial.println(F("All sensors initialized successfully!"));
   Serial.println(F("Waiting for flight conditions..."));
   
   // Initialize timing
   lastSampleTime = millis();
   
   // Final startup indication
   blinkLED(5); // 5 blinks = ready
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
 
     // Blink LED to show activity
     digitalWrite(STATUS_LED_PIN, HIGH);
     delay(50);
     digitalWrite(STATUS_LED_PIN, LOW);
   }
 
   // Small delay to prevent overwhelming the system
   delay(10);
 }
 
 void initializeAllSensors() {
   Serial.println(F("\n=== Sensor Initialization ==="));
 
   // Try to initialize SD card (optional in test mode)
   Serial.println(F("Initializing SD card..."));
   sdCardAvailable = initSD();
   if (sdCardAvailable) {
     Serial.println(F(" SD card initialized successfully"));
     // Log initialization header
     String header = "Timestamp,Pressure_Pa,Altitude_m,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,FlightState";
     logData(header);
   } else {
     Serial.println(F("  SD card initialization failed - running in test mode"));
     Serial.println(F("   Check: CS pin 9, MOSI pin 11, MISO pin 12, SCK pin 13"));
   }
 
   // Initialize IMU (placeholder - implement when imu.h/cpp are ready)
   // initIMU();
 
   // Initialize barometer (placeholder - implement when baro.h/cpp are ready)
   // initBaro();
 
   Serial.println(F("=== Initialization Complete ===\n"));
 }
 
 void collectTelemetryData() {
   currentData.timestamp = millis();
   currentData.state = currentState;
 
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
       blinkLED(10); // 10 blinks = flight detected
     } else if (flightStarted && currentAltitude < lastAltitude - ALTITUDE_FALL_THRESHOLD_M) {
       // Altitude decreased significantly - likely landing
       currentState = POSTFLIGHT;
       Serial.println(F("LANDING DETECTED"));
       blinkLED(15); // 15 blinks = landing detected
     }
     
     lastAltitude = currentAltitude;
     lastAltitudeCheck = millis();
   }
 }
 
 void logTelemetryData() {
   if (sdCardAvailable) {
     String dataString = formatTelemetryString();
     if (!logData(dataString)) {
       Serial.println(F("ERROR: Failed to log data to SD card"));
     }
   }
 }
 
 String formatTelemetryString() {
   String dataString = "";
   dataString += currentData.timestamp;
   dataString += ",";
   dataString += currentData.pressure;
   dataString += ",";
   dataString += currentData.altitude;
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
   Serial.print(F("Alt: "));
   Serial.print(currentData.altitude);
   Serial.print(F("m, Press: "));
   Serial.print(currentData.pressure);
   Serial.print(F("Pa, State: "));
 
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
 
 void blinkLED(int times) {
   for (int i = 0; i < times; i++) {
     digitalWrite(STATUS_LED_PIN, HIGH);
     delay(200);
     digitalWrite(STATUS_LED_PIN, LOW);
     delay(200);
   }
 }