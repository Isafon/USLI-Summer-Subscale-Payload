#include "sequencer.h"
#include "flight_detection.h"
#include "hardware_control.h"
#include "config.h"
#include "temp.h"
#include "gps.h"
#include "uSD.h"

// Global sequencer state
SequencerControl sequencer;
SequencerTelemetryData sequencerData;

void initSequencer() {
  sequencer.currentState = SBIT_INIT_SEQ_IMU;
  sequencer.currentPhase = PHASE_STARTUP;
  sequencer.stateStartTime = millis();
  sequencer.lastStateUpdate = millis();
  sequencer.sequenceActive = true;
  sequencer.emergencyAbort = false;
  sequencer.sequenceRetries = 0;
  sequencer.launchAltitude = 0.0;
  sequencer.maxAltitude = 0.0;
  sequencer.apogeeDetected = false;
  
  Serial.println(F("Sequencer initialized - Beginning SBIT sequence"));
}

void updateSequencer() {
  if (!sequencer.sequenceActive && !sequencer.emergencyAbort) {
    return; // Sequencer is paused
  }
  
  if (sequencer.emergencyAbort) {
    executeAbortSequence();
    return;
  }
  
  // Check for state timeout
  if (millis() - sequencer.stateStartTime > STATE_TIMEOUT_MS) {
    Serial.print(F("WARNING: State timeout for "));
    Serial.println(getStateString(sequencer.currentState));
    
    // Handle timeout based on current state
    if (sequencer.currentState <= SBIT_RBSAFE_CHECK) {
      // Startup phase timeout - retry or abort
      sequencer.sequenceRetries++;
      if (sequencer.sequenceRetries > 3) {
        Serial.println(F("ABORT: Too many startup failures"));
        sequencer.emergencyAbort = true;
        return;
      }
    }
  }
  
  // Execute current state logic
  executeCurrentState();
}

void executeCurrentState() {
  switch (sequencer.currentState) {
    case SBIT_INIT_SEQ_IMU:
      Serial.println(F("SBIT-0: Initializing Sequencer & IMU"));
      // Initialize IMU when imu.cpp is implemented
      // For now, just transition after sensor checks
      if (checkSensorStatus()) {
        transitionToState(SBIT_STARTUP_BATTERY);
      }
      break;
      
    case SBIT_STARTUP_BATTERY:
      Serial.println(F("SBIT-1: Checking Startup Battery"));
      checkBatteryStatus();
      if (sequencerData.batteryOK) {
        transitionToState(SBIT_STARTUP_TELEM);
      }
      break;
      
    case SBIT_STARTUP_TELEM:
      Serial.println(F("SBIT-2: Starting Telemetry Systems"));
      // Telemetry already running, verify all sensors
      if (sequencerData.sensorsOK) {
        transitionToState(SBIT_STARTUP_PAYLOAD);
      }
      break;
      
    case SBIT_STARTUP_PAYLOAD:
      Serial.println(F("SBIT-3: Checking Payload Battery"));
      enablePayloadPower();
      // Add payload battery check here
      sequencerData.payloadOK = true; // Placeholder
      if (sequencerData.payloadOK) {
        transitionToState(SBIT_PAYLOAD_TELEM);
      }
      break;
      
    case SBIT_PAYLOAD_TELEM:
      Serial.println(F("SBIT-4: Starting Payload Telemetry"));
      // Verify payload sensors are responding
      if (sequencerData.payloadOK && sequencerData.sensorsOK) {
        transitionToState(SBIT_RBSAFE_CHECK);
      }
      break;
      
    case SBIT_RBSAFE_CHECK:
      Serial.println(F("SBIT-5: RBSAFE Validation Check"));
      // Perform final safety checks
      if (sequencerData.batteryOK && sequencerData.sensorsOK && sequencerData.payloadOK) {
        Serial.println(F("SBIT Complete - Ready for Launch Sequence"));
        transitionToState(LBIT_IGNIT_BOOSTER);
        transitionToPhase(PHASE_PREFLIGHT);
      }
      break;
      
    case LBIT_IGNIT_BOOSTER:
      Serial.println(F("LBIT-6: Booster Ignition Ready"));
      // Wait for launch detection via accelerometer
      if (sequencerData.accel_magnitude > LAUNCH_ACCEL_THRESHOLD_G) {
        igniteBooster();
        sequencer.launchAltitude = sequencerData.altitudeAGL;
        transitionToState(LBIT_LAUNCH);
        transitionToPhase(PHASE_LAUNCH);
      }
      break;
      
    case LBIT_LAUNCH:
      Serial.println(F("LBIT-7: LAUNCH CONFIRMED!"));
      // Monitor initial ascent
      if (sequencerData.altitudeAGL > sequencer.launchAltitude + 50.0) {
        transitionToState(LBIT_POST_LAUNCH_REPORT);
        transitionToPhase(PHASE_FLIGHT);
      }
      break;
      
    case LBIT_POST_LAUNCH_REPORT:
      Serial.println(F("LBIT-8: Post-Launch Report"));
      sendTelemetryBurst();
      // Track for apogee
      if (sequencerData.altitude > sequencer.maxAltitude) {
        sequencer.maxAltitude = sequencerData.altitude;
      }
      if (sequencerData.vertical_velocity < APOGEE_VELOCITY_THRESHOLD) {
        sequencer.apogeeDetected = true;
        transitionToState(LBIT_APOGEE_REPORT);
      }
      break;
      
    case LBIT_APOGEE_REPORT:
      Serial.println(F("LBIT-9: Apogee Detected - Reporting"));
      Serial.print(F("Maximum Altitude: "));
      Serial.print(sequencer.maxAltitude);
      Serial.println(F(" meters"));
      sendTelemetryBurst();
      transitionToState(DBIT_POP_NOSE_FAIRING);
      transitionToPhase(PHASE_DEPLOY);
      break;
      
    case DBIT_POP_NOSE_FAIRING:
      Serial.println(F("DBIT-10: Popping Nose Fairing"));
      popNoseFairing();
      delay(2000); // Allow fairing to clear
      transitionToState(DBIT_STAGE_SEPARATION);
      break;
      
    case DBIT_STAGE_SEPARATION:
      Serial.println(F("DBIT-11: Stage Separation"));
      separateStage();
      delay(3000); // Allow separation
      transitionToState(DBIT_BOOM_PAYLOAD);
      break;
      
    case DBIT_BOOM_PAYLOAD:
      Serial.println(F("DBIT-12: Deploying Payload"));
      deployPayload();
      transitionToState(DBIT_FINAL_MODE);
      break;
      
    case DBIT_FINAL_MODE:
      Serial.println(F("DBIT-13: Final Deploy Mode"));
      // Wait a bit before parachute deployment
      if (millis() - sequencer.stateStartTime > 5000) {
        transitionToState(DBIT_DEPLOY_PARACHUTE);
      }
      break;
      
    case DBIT_DEPLOY_PARACHUTE:
      Serial.println(F("DBIT-14: Deploying Parachute"));
      deployParachute();
      transitionToState(DBIT_SEND_ALL_TELEM);
      transitionToPhase(PHASE_RECOVERY);
      break;
      
    case DBIT_SEND_ALL_TELEM:
      Serial.println(F("DBIT-15: Sending All Recorded Telemetry"));
      sendTelemetryBurst();
      // Continue logging until shutdown
      if (sequencerData.vertical_velocity < LANDING_VELOCITY_THRESHOLD && 
          sequencerData.accel_magnitude < 1.5) {
        // Likely landed, prepare for shutdown
        delay(30000); // Log for 30 more seconds
        transitionToState(ABIT_KILL_ALL_PROCESSES);
      }
      break;
      
    case ABIT_KILL_ALL_PROCESSES:
      Serial.println(F("ABIT-16: Mission Complete - Shutting Down"));
      // Final telemetry burst
      sendTelemetryBurst();
      // Safe all pyrotechnics
      safeAllPyrotechnics();
      
      sequencer.sequenceActive = false;
      Serial.println(F("=== MISSION COMPLETE ==="));
      break;
  }
}

void transitionToState(SequencerState newState) {
  Serial.print(F("State Transition: "));
  Serial.print(getStateString(sequencer.currentState));
  Serial.print(F(" -> "));
  Serial.println(getStateString(newState));
  
  sequencer.currentState = newState;
  sequencer.stateStartTime = millis();
  sequencer.sequenceRetries = 0; // Reset retry counter on successful transition
}

void transitionToPhase(MissionPhase newPhase) {
  Serial.print(F("Phase Transition: "));
  Serial.print(getPhaseString(sequencer.currentPhase));
  Serial.print(F(" -> "));
  Serial.println(getPhaseString(newPhase));
  
  sequencer.currentPhase = newPhase;
}

void checkEmergencyConditions() {
  // Check emergency abort pin
  if (digitalRead(EMERGENCY_ABORT_PIN) == LOW) {
    Serial.println(F("EMERGENCY ABORT ACTIVATED"));
    sequencer.emergencyAbort = true;
    return;
  }
  
  // Check for critical battery failure
  if (!sequencerData.batteryOK && sequencer.currentPhase >= PHASE_LAUNCH) {
    Serial.println(F("CRITICAL BATTERY FAILURE - EMERGENCY ACTIONS"));
    // Deploy parachute immediately if in flight
    if (sequencer.currentPhase == PHASE_FLIGHT) {
      deployParachute();
    }
  }
}

void executeAbortSequence() {
  Serial.println(F("EXECUTING EMERGENCY ABORT SEQUENCE"));
  
  // Safe all pyrotechnics
  safeAllPyrotechnics();
  
  // Deploy recovery systems if in flight
  if (sequencer.currentPhase >= PHASE_LAUNCH) {
    deployParachute();
  }
  
  // Send emergency telemetry
  sendTelemetryBurst();
  
  // Transition to shutdown
  transitionToState(ABIT_KILL_ALL_PROCESSES);
}

void collectSequencerTelemetry() {
  // Update sequencer data
  sequencerData.timestamp = millis();
  sequencerData.sequencerState = sequencer.currentState;
  sequencerData.missionPhase = sequencer.currentPhase;
  
  // Read temperature
  sequencerData.temperature = readTemperatureC();
  
  // Read GPS data
  if (gps.location.isValid()) {
    sequencerData.gps_lat = gps.location.lat();
    sequencerData.gps_lon = gps.location.lng();
  } else {
    sequencerData.gps_lat = 0.0;
    sequencerData.gps_lon = 0.0;
  }
  
  if (gps.altitude.isValid()) {
    sequencerData.gps_alt = gps.altitude.meters();
  } else {
    sequencerData.gps_alt = 0.0;
  }
  
  sequencerData.gps_satellites = gps.satellites.value();
  
  // Placeholder for barometer and IMU data
  sequencerData.pressure = 0.0;
  sequencerData.altitude = sequencerData.gps_alt; // Use GPS altitude for now
  sequencerData.altitudeAGL = sequencerData.altitude - sequencer.launchAltitude;
  
  // Placeholder IMU data - implement when imu.cpp is ready
  sequencerData.accel_x = 0.0;
  sequencerData.accel_y = 0.0; 
  sequencerData.accel_z = 1.0; // 1G gravity baseline
  sequencerData.gyro_x = 0.0;
  sequencerData.gyro_y = 0.0;
  sequencerData.gyro_z = 0.0;
  
  // Update derived values
  updateFlightDetection();
  checkBatteryStatus();
}

void logSequencerData() {
  if (sdCardAvailable) {
    String dataString = formatSequencerTelemetry();
    if (!logData(dataString)) {
      Serial.println(F("ERROR: Failed to log sequencer data"));
    }
  }
}

String formatSequencerTelemetry() {
  String dataString = "";
  dataString += sequencerData.timestamp;
  dataString += ",";
  dataString += (int)sequencerData.sequencerState;
  dataString += ",";
  dataString += getStateString(sequencerData.sequencerState);
  dataString += ",";
  dataString += (int)sequencerData.missionPhase;
  dataString += ",";
  dataString += getPhaseString(sequencerData.missionPhase);
  dataString += ",";
  dataString += sequencerData.temperature;
  dataString += ",";
  dataString += sequencerData.pressure;
  dataString += ",";
  dataString += sequencerData.altitude;
  dataString += ",";
  dataString += sequencerData.altitudeAGL;
  dataString += ",";
  dataString += sequencerData.vertical_velocity;
  dataString += ",";
  dataString += String(sequencerData.gps_lat, 6);
  dataString += ",";
  dataString += String(sequencerData.gps_lon, 6);
  dataString += ",";
  dataString += sequencerData.gps_alt;
  dataString += ",";
  dataString += sequencerData.gps_satellites;
  dataString += ",";
  dataString += sequencerData.accel_x;
  dataString += ",";
  dataString += sequencerData.accel_y;
  dataString += ",";
  dataString += sequencerData.accel_z;
  dataString += ",";
  dataString += sequencerData.accel_magnitude;
  dataString += ",";
  dataString += sequencerData.gyro_x;
  dataString += ",";
  dataString += sequencerData.gyro_y;
  dataString += ",";
  dataString += sequencerData.gyro_z;
  dataString += ",";
  dataString += sequencerData.batteryOK ? "OK" : "LOW";
  dataString += ",";
  dataString += sequencerData.sensorsOK ? "OK" : "FAIL";
  dataString += ",";
  dataString += sequencerData.payloadOK ? "OK" : "FAIL";
  
  return dataString;
}

void sendTelemetryBurst() {
  // Send critical telemetry data
  Serial.println(F("=== TELEMETRY BURST ==="));
  Serial.print(F("State: "));
  Serial.println(getStateString(sequencer.currentState));
  Serial.print(F("Phase: "));
  Serial.println(getPhaseString(sequencer.currentPhase));
  Serial.print(F("Altitude: "));
  Serial.print(sequencerData.altitude);
  Serial.println(F(" m"));
  Serial.print(F("AGL: "));
  Serial.print(sequencerData.altitudeAGL);
  Serial.println(F(" m"));
  Serial.print(F("Vertical Velocity: "));
  Serial.print(sequencerData.vertical_velocity);
  Serial.println(F(" m/s"));
  Serial.print(F("Acceleration: "));
  Serial.print(sequencerData.accel_magnitude);
  Serial.println(F(" G"));
  Serial.println(F("====================="));
}

void blinkSequencerStatus() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  // Blink pattern based on current phase
  unsigned long blinkRate = 1000; // Default 1Hz
  
  switch (sequencer.currentPhase) {
    case PHASE_STARTUP:
      blinkRate = 2000; // Slow blink
      break;
    case PHASE_PREFLIGHT:
      blinkRate = 500; // Medium blink
      break;
    case PHASE_LAUNCH:
    case PHASE_FLIGHT:
      blinkRate = 100; // Fast blink
      break;
    case PHASE_DEPLOY:
      blinkRate = 200; // Fast blink
      break;
    case PHASE_RECOVERY:
      blinkRate = 1000; // Slow blink
      break;
    case PHASE_ABORT:
      blinkRate = 50; // Very fast blink
      break;
  }
  
  if (millis() - lastBlink >= blinkRate) {
    ledState = !ledState;
    digitalWrite(STATUS_LED_PIN, ledState);
    lastBlink = millis();
  }
}

String getStateString(SequencerState state) {
  switch (state) {
    case SBIT_INIT_SEQ_IMU: return "SBIT-0: Init Seq/IMU";
    case SBIT_STARTUP_BATTERY: return "SBIT-1: Startup Battery";
    case SBIT_STARTUP_TELEM: return "SBIT-2: Startup Telemetry";
    case SBIT_STARTUP_PAYLOAD: return "SBIT-3: Startup Payload";
    case SBIT_PAYLOAD_TELEM: return "SBIT-4: Payload Telemetry";
    case SBIT_RBSAFE_CHECK: return "SBIT-5: RBSAFE Check";
    case LBIT_IGNIT_BOOSTER: return "LBIT-6: Ignit Booster";
    case LBIT_LAUNCH: return "LBIT-7: Launch";
    case LBIT_POST_LAUNCH_REPORT: return "LBIT-8: Post Launch";
    case LBIT_APOGEE_REPORT: return "LBIT-9: Apogee Report";
    case DBIT_POP_NOSE_FAIRING: return "DBIT-10: Pop Nose Fairing";
    case DBIT_STAGE_SEPARATION: return "DBIT-11: Stage Separation";
    case DBIT_BOOM_PAYLOAD: return "DBIT-12: Boom Payload";
    case DBIT_FINAL_MODE: return "DBIT-13: Final Mode";
    case DBIT_DEPLOY_PARACHUTE: return "DBIT-14: Deploy Parachute";
    case DBIT_SEND_ALL_TELEM: return "DBIT-15: Send All Telemetry";
    case ABIT_KILL_ALL_PROCESSES: return "ABIT-16: Kill All Processes";
    default: return "UNKNOWN";
  }
}

String getPhaseString(MissionPhase phase) {
  switch (phase) {
    case PHASE_STARTUP: return "STARTUP";
    case PHASE_PREFLIGHT: return "PREFLIGHT";
    case PHASE_LAUNCH: return "LAUNCH";
    case PHASE_FLIGHT: return "FLIGHT";
    case PHASE_DEPLOY: return "DEPLOY";
    case PHASE_RECOVERY: return "RECOVERY";
    case PHASE_ABORT: return "ABORT";
    default: return "UNKNOWN";
  }
}