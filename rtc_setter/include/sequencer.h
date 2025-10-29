#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <Arduino.h>

// Sequencer State Definitions
enum SequencerState {
  // SBIT - Startup Sequence (0-5)
  SBIT_INIT_SEQ_IMU = 0,        // Initialize sequencer & IMU
  SBIT_STARTUP_BATTERY = 1,     // Check startup battery
  SBIT_STARTUP_TELEM = 2,       // Startup telemetry (sensors)
  SBIT_STARTUP_PAYLOAD = 3,     // Startup payload battery
  SBIT_PAYLOAD_TELEM = 4,       // Startup payload telemetry
  SBIT_RBSAFE_CHECK = 5,        // Check RBSAFE validation
  
  // LBIT - Launch Sequence (6-9)
  LBIT_IGNIT_BOOSTER = 6,       // Ignit booster
  LBIT_LAUNCH = 7,              // Launch!!!
  LBIT_POST_LAUNCH_REPORT = 8,  // Post-launch report bit (reports apogee)
  LBIT_APOGEE_REPORT = 9,       // Reports apogee via mag alt pages
  
  // DBIT - Deploy Sequence (10-15)
  DBIT_POP_NOSE_FAIRING = 10,   // Pop nose fairing
  DBIT_STAGE_SEPARATION = 11,   // First stage separation
  DBIT_BOOM_PAYLOAD = 12,       // Boom payload from booster
  DBIT_FINAL_MODE = 13,         // Final mode (post deploy)
  DBIT_DEPLOY_PARACHUTE = 14,   // From deploy parachute
  DBIT_SEND_ALL_TELEM = 15,     // Send all recorded telemetry
  
  // ABIT - Abort Sequence (16)
  ABIT_KILL_ALL_PROCESSES = 16  // Kills all processes & shuts down
};

// Mission Phase tracking for higher-level state management
enum MissionPhase {
  PHASE_STARTUP,
  PHASE_PREFLIGHT,
  PHASE_LAUNCH,
  PHASE_FLIGHT,
  PHASE_DEPLOY,
  PHASE_RECOVERY,
  PHASE_ABORT
};

// Sequencer control structure
struct SequencerControl {
  SequencerState currentState;
  MissionPhase currentPhase;
  unsigned long stateStartTime;
  unsigned long lastStateUpdate;
  bool sequenceActive;
  bool emergencyAbort;
  int sequenceRetries;
  float launchAltitude;
  float maxAltitude;
  bool apogeeDetected;
};

// Enhanced telemetry data structure
struct SequencerTelemetryData {
  unsigned long timestamp;
  SequencerState sequencerState;
  MissionPhase missionPhase;
  float temperature;
  float pressure;
  float altitude;
  float altitudeAGL;  // Above Ground Level
  float gps_lat;
  float gps_lon;
  float gps_alt;
  int gps_satellites;
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float accel_magnitude;
  float vertical_velocity;
  bool batteryOK;
  bool sensorsOK;
  bool payloadOK;
};

// Function prototypes
void initSequencer();
void updateSequencer();
void executeCurrentState();
void transitionToState(SequencerState newState);
void transitionToPhase(MissionPhase newPhase);
void checkEmergencyConditions();
void executeAbortSequence();

// Telemetry functions
void collectSequencerTelemetry();
void logSequencerData();
String formatSequencerTelemetry();
void sendTelemetryBurst();

// Utility functions
void blinkSequencerStatus();
String getStateString(SequencerState state);
String getPhaseString(MissionPhase phase);

// External variables (defined in sequencer.cpp)
extern SequencerControl sequencer;
extern SequencerTelemetryData sequencerData;
extern bool sdCardAvailable;

#endif // SEQUENCER_H