#include "flight_detection.h"
#include "sequencer.h"
#include <math.h>

void updateFlightDetection() {
  // Calculate vertical velocity (simplified)
  sequencerData.vertical_velocity = calculateVerticalVelocity();
  
  // Calculate acceleration magnitude
  sequencerData.accel_magnitude = calculateAccelerationMagnitude();
}

bool detectLaunch() {
  return (sequencerData.accel_magnitude > LAUNCH_ACCEL_THRESHOLD_G);
}

bool detectApogee() {
  return (sequencerData.vertical_velocity < APOGEE_VELOCITY_THRESHOLD);
}

bool detectLanding() {
  return (sequencerData.vertical_velocity < LANDING_VELOCITY_THRESHOLD && 
          sequencerData.accel_magnitude < 1.5);
}

float calculateVerticalVelocity() {
  static float lastAltitude = 0.0;
  static unsigned long lastAltTime = 0;
  
  if (millis() - lastAltTime >= 1000) { // Update every second
    float deltaAlt = sequencerData.altitude - lastAltitude;
    float deltaTime = (millis() - lastAltTime) / 1000.0;
    float velocity = deltaAlt / deltaTime;
    
    lastAltitude = sequencerData.altitude;
    lastAltTime = millis();
    
    return velocity;
  }
  
  return sequencerData.vertical_velocity; // Return previous value if not time to update
}

float calculateAccelerationMagnitude() {
  return sqrt(
    sequencerData.accel_x * sequencerData.accel_x +
    sequencerData.accel_y * sequencerData.accel_y +
    sequencerData.accel_z * sequencerData.accel_z
  );
}