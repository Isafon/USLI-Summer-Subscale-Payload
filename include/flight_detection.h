#ifndef FLIGHT_DETECTION_H
#define FLIGHT_DETECTION_H

#include <Arduino.h>

// Flight detection thresholds
#define LAUNCH_ACCEL_THRESHOLD_G 2.0   // G-force to detect launch
#define APOGEE_VELOCITY_THRESHOLD -2.0 // Negative velocity for apogee
#define LANDING_VELOCITY_THRESHOLD 5.0 // Low velocity for landing detection
#define MINIMUM_FLIGHT_ALTITUDE_M 30.0 // Minimum altitude to be considered flight

// Timing constants
#define STATE_TIMEOUT_MS 30000        // 30 second state timeout
#define SEQUENCER_UPDATE_RATE_MS 100  // 10Hz sequencer updates
#define FAST_TELEM_RATE_MS 50        // 20Hz during critical phases
#define NORMAL_TELEM_RATE_MS 200     // 5Hz during normal operations

// Flight detection functions
void updateFlightDetection();
bool detectLaunch();
bool detectApogee();
bool detectLanding();
float calculateVerticalVelocity();
float calculateAccelerationMagnitude();

#endif // FLIGHT_DETECTION_H