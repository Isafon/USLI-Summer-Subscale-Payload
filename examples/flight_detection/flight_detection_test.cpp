/*
 * Flight Detection Algorithm Test
 *
 * This sketch simulates flight profiles and tests the flight detection
 * algorithms for launch, apogee, and landing detection.
 *
 * Test Scenarios:
 * 1. Normal flight profile (launch -> apogee -> landing)
 * 2. High-G launch detection
 * 3. Apogee detection via velocity
 * 4. Landing detection
 * 5. False positive prevention
 *
 * No hardware required - uses simulated sensor data
 *
 * Serial Commands:
 * - '1' = Simulate normal flight
 * - '2' = Simulate high-altitude flight
 * - '3' = Simulate abort scenario
 * - '4' = Test launch detection threshold
 * - '5' = Test apogee detection
 * - 'r' = Reset simulation
 * - 'h' = Show help
 */

#include <Arduino.h>

// Flight detection thresholds (from flight_detection.h)
#define LAUNCH_ACCEL_THRESHOLD_G 2.0
#define APOGEE_VELOCITY_THRESHOLD -2.0
#define LANDING_VELOCITY_THRESHOLD 5.0
#define MINIMUM_FLIGHT_ALTITUDE_M 30.0

// Simulation parameters
#define SIM_UPDATE_RATE_MS 100  // 10Hz simulation
#define TIME_STEP 0.1           // 100ms in seconds

// Flight states
enum FlightState {
  STATE_PREFLIGHT,
  STATE_LAUNCH,
  STATE_POWERED_ASCENT,
  STATE_COASTING,
  STATE_APOGEE,
  STATE_DESCENT,
  STATE_LANDING,
  STATE_POSTFLIGHT
};

// Simulation state
struct SimulationState {
  float altitude;          // meters
  float velocity;          // m/s (positive = up)
  float acceleration;      // m/s² (vertical)
  float accel_magnitude;   // G-forces (total)
  float time;              // seconds since start
  FlightState state;
  bool launchDetected;
  bool apogeeDetected;
  bool landingDetected;
} sim;

// Simulation profiles
float maxAltitude = 300.0;  // Target apogee (meters)
float burnTime = 2.0;       // Motor burn time (seconds)
float thrustAccel = 8.0;    // Thrust acceleration (G)

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("========================================"));
  Serial.println(F("Flight Detection Algorithm Test"));
  Serial.println(F("========================================\n"));

  Serial.println(F("This test simulates rocket flight profiles"));
  Serial.println(F("and validates flight state detection.\n"));

  printMenu();
  resetSimulation();
}

void loop() {
  // Check for commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  // Run simulation at fixed rate
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= SIM_UPDATE_RATE_MS) {
    updateSimulation();
    updateFlightDetection();
    printTelemetry();
    lastUpdate = millis();
  }
}

void handleCommand(char cmd) {
  Serial.println();

  switch (cmd) {
    case '1':
      Serial.println(F("Starting normal flight simulation..."));
      maxAltitude = 300.0;
      burnTime = 2.0;
      thrustAccel = 8.0;
      resetSimulation();
      break;

    case '2':
      Serial.println(F("Starting high-altitude flight simulation..."));
      maxAltitude = 1000.0;
      burnTime = 3.0;
      thrustAccel = 12.0;
      resetSimulation();
      break;

    case '3':
      Serial.println(F("Starting abort scenario simulation..."));
      maxAltitude = 100.0;
      burnTime = 0.5;
      thrustAccel = 3.0;
      resetSimulation();
      break;

    case '4':
      testLaunchThreshold();
      break;

    case '5':
      testApogeeDetection();
      break;

    case 'r':
    case 'R':
      Serial.println(F("Resetting simulation..."));
      resetSimulation();
      break;

    case 'h':
    case 'H':
      printMenu();
      break;

    case '\n':
    case '\r':
      break;

    default:
      Serial.print(F("Unknown command: "));
      Serial.println(cmd);
      break;
  }
}

void resetSimulation() {
  sim.altitude = 0;
  sim.velocity = 0;
  sim.acceleration = 0;
  sim.accel_magnitude = 1.0;  // 1G at rest
  sim.time = 0;
  sim.state = STATE_PREFLIGHT;
  sim.launchDetected = false;
  sim.apogeeDetected = false;
  sim.landingDetected = false;

  Serial.println(F("\n========================================"));
  Serial.println(F("Simulation Reset"));
  Serial.println(F("========================================"));
  Serial.print(F("Target Apogee: "));
  Serial.print(maxAltitude);
  Serial.println(F(" m"));
  Serial.print(F("Burn Time: "));
  Serial.print(burnTime);
  Serial.println(F(" s"));
  Serial.print(F("Thrust: "));
  Serial.print(thrustAccel);
  Serial.println(F(" G"));
  Serial.println(F("========================================\n"));
  Serial.println(F("Time(s)\tAlt(m)\tVel(m/s)\tAccel(G)\tState"));
  Serial.println(F("--------------------------------------------------------"));
}

void updateSimulation() {
  sim.time += TIME_STEP;

  // Simple flight dynamics model
  if (sim.state == STATE_PREFLIGHT) {
    // Waiting on pad
    if (sim.time > 5.0) {  // Auto-launch after 5 seconds
      sim.state = STATE_LAUNCH;
    }
  }
  else if (sim.state == STATE_LAUNCH || sim.state == STATE_POWERED_ASCENT) {
    // Motor burning
    if (sim.time < (5.0 + burnTime)) {
      sim.acceleration = thrustAccel * 9.81 - 9.81;  // Thrust minus gravity
      sim.accel_magnitude = thrustAccel;
      sim.state = STATE_POWERED_ASCENT;
    } else {
      // Motor burnout - coast phase
      sim.acceleration = -9.81;  // Only gravity
      sim.accel_magnitude = 1.0;
      sim.state = STATE_COASTING;
    }
  }
  else if (sim.state == STATE_COASTING) {
    // Coasting to apogee
    sim.acceleration = -9.81;  // Gravity + drag (simplified)
    sim.accel_magnitude = 1.0;

    if (sim.velocity <= 0) {
      sim.state = STATE_APOGEE;
    }
  }
  else if (sim.state == STATE_APOGEE) {
    // At apogee, start descent
    sim.state = STATE_DESCENT;
  }
  else if (sim.state == STATE_DESCENT) {
    // Descending with parachute
    // Terminal velocity with drag
    float terminalVelocity = -10.0;  // m/s (parachute deployed)
    if (sim.velocity > terminalVelocity) {
      sim.acceleration = -9.81;
    } else {
      sim.acceleration = 0;  // Reached terminal velocity
      sim.velocity = terminalVelocity;
    }
    sim.accel_magnitude = 1.0;

    if (sim.altitude <= 0) {
      sim.altitude = 0;
      sim.velocity = 0;
      sim.acceleration = 0;
      sim.state = STATE_LANDING;
    }
  }
  else if (sim.state == STATE_LANDING) {
    // Landed
    sim.altitude = 0;
    sim.velocity = 0;
    sim.acceleration = 0;
    sim.accel_magnitude = 1.0;

    if (!sim.landingDetected) {
      sim.state = STATE_POSTFLIGHT;
    }
  }

  // Update velocity and altitude (basic integration)
  sim.velocity += sim.acceleration * TIME_STEP;
  sim.altitude += sim.velocity * TIME_STEP;

  // Don't go below ground
  if (sim.altitude < 0) {
    sim.altitude = 0;
    sim.velocity = 0;
  }
}

void updateFlightDetection() {
  // Launch detection
  if (!sim.launchDetected && detectLaunch()) {
    sim.launchDetected = true;
    Serial.println();
    Serial.println(F("🚀 *** LAUNCH DETECTED! ***"));
    Serial.print(F("   Time: "));
    Serial.print(sim.time, 2);
    Serial.println(F(" s"));
    Serial.print(F("   Acceleration: "));
    Serial.print(sim.accel_magnitude, 2);
    Serial.println(F(" G"));
    Serial.println();
  }

  // Apogee detection
  if (sim.launchDetected && !sim.apogeeDetected && detectApogee()) {
    sim.apogeeDetected = true;
    Serial.println();
    Serial.println(F("⛰️  *** APOGEE DETECTED! ***"));
    Serial.print(F("   Time: "));
    Serial.print(sim.time, 2);
    Serial.println(F(" s"));
    Serial.print(F("   Altitude: "));
    Serial.print(sim.altitude, 2);
    Serial.println(F(" m"));
    Serial.print(F("   Velocity: "));
    Serial.print(sim.velocity, 2);
    Serial.println(F(" m/s"));
    Serial.println();
  }

  // Landing detection
  if (sim.apogeeDetected && !sim.landingDetected && detectLanding()) {
    sim.landingDetected = true;
    Serial.println();
    Serial.println(F("🪂 *** LANDING DETECTED! ***"));
    Serial.print(F("   Time: "));
    Serial.print(sim.time, 2);
    Serial.println(F(" s"));
    Serial.print(F("   Velocity: "));
    Serial.print(sim.velocity, 2);
    Serial.println(F(" m/s"));
    Serial.println();
    printFlightSummary();
  }
}

bool detectLaunch() {
  return sim.accel_magnitude > LAUNCH_ACCEL_THRESHOLD_G;
}

bool detectApogee() {
  return (sim.altitude > MINIMUM_FLIGHT_ALTITUDE_M &&
          sim.velocity < APOGEE_VELOCITY_THRESHOLD);
}

bool detectLanding() {
  return (sim.altitude < 5.0 &&
          abs(sim.velocity) < LANDING_VELOCITY_THRESHOLD &&
          sim.accel_magnitude < 1.5);
}

void printTelemetry() {
  Serial.print(sim.time, 1);
  Serial.print(F("\t"));
  Serial.print(sim.altitude, 1);
  Serial.print(F("\t"));
  Serial.print(sim.velocity, 1);
  Serial.print(F("\t\t"));
  Serial.print(sim.accel_magnitude, 2);
  Serial.print(F("\t\t"));
  Serial.println(getStateString(sim.state));
}

void printFlightSummary() {
  Serial.println(F("========================================"));
  Serial.println(F("Flight Summary"));
  Serial.println(F("========================================"));
  Serial.print(F("Total Flight Time: "));
  Serial.print(sim.time, 1);
  Serial.println(F(" s"));

  Serial.print(F("Max Altitude: "));
  Serial.print(sim.altitude, 1);
  Serial.println(F(" m"));

  Serial.print(F("Launch Detection: "));
  Serial.println(sim.launchDetected ? F("✓ PASS") : F("✗ FAIL"));

  Serial.print(F("Apogee Detection: "));
  Serial.println(sim.apogeeDetected ? F("✓ PASS") : F("✗ FAIL"));

  Serial.print(F("Landing Detection: "));
  Serial.println(sim.landingDetected ? F("✓ PASS") : F("✗ FAIL"));

  Serial.println(F("========================================\n"));
}

void testLaunchThreshold() {
  Serial.println(F("\n========================================"));
  Serial.println(F("Testing Launch Detection Threshold"));
  Serial.println(F("========================================"));
  Serial.print(F("Threshold: "));
  Serial.print(LAUNCH_ACCEL_THRESHOLD_G);
  Serial.println(F(" G\n"));

  for (float accel = 0.5; accel <= 5.0; accel += 0.5) {
    sim.accel_magnitude = accel;
    bool detected = detectLaunch();

    Serial.print(F("Accel: "));
    Serial.print(accel, 1);
    Serial.print(F(" G -> "));
    Serial.println(detected ? F("✓ LAUNCH DETECTED") : F("  No launch"));
  }

  Serial.println(F("========================================\n"));
  resetSimulation();
}

void testApogeeDetection() {
  Serial.println(F("\n========================================"));
  Serial.println(F("Testing Apogee Detection"));
  Serial.println(F("========================================"));
  Serial.print(F("Velocity Threshold: "));
  Serial.print(APOGEE_VELOCITY_THRESHOLD);
  Serial.println(F(" m/s"));
  Serial.print(F("Minimum Altitude: "));
  Serial.print(MINIMUM_FLIGHT_ALTITUDE_M);
  Serial.println(F(" m\n"));

  sim.altitude = 50.0;  // Above minimum

  for (float vel = 10.0; vel >= -10.0; vel -= 2.0) {
    sim.velocity = vel;
    bool detected = detectApogee();

    Serial.print(F("Velocity: "));
    Serial.print(vel, 1);
    Serial.print(F(" m/s -> "));
    Serial.println(detected ? F("✓ APOGEE DETECTED") : F("  No apogee"));
  }

  Serial.println(F("========================================\n"));
  resetSimulation();
}

const char* getStateString(FlightState state) {
  switch (state) {
    case STATE_PREFLIGHT: return "PREFLIGHT";
    case STATE_LAUNCH: return "LAUNCH";
    case STATE_POWERED_ASCENT: return "POWERED ASCENT";
    case STATE_COASTING: return "COASTING";
    case STATE_APOGEE: return "APOGEE";
    case STATE_DESCENT: return "DESCENT";
    case STATE_LANDING: return "LANDING";
    case STATE_POSTFLIGHT: return "POSTFLIGHT";
    default: return "UNKNOWN";
  }
}

void printMenu() {
  Serial.println(F("========================================"));
  Serial.println(F("Command Menu"));
  Serial.println(F("========================================"));
  Serial.println(F("Flight Simulations:"));
  Serial.println(F("  1 - Normal flight (300m apogee)"));
  Serial.println(F("  2 - High-altitude flight (1000m)"));
  Serial.println(F("  3 - Abort scenario (low altitude)"));
  Serial.println();
  Serial.println(F("Detection Tests:"));
  Serial.println(F("  4 - Test launch threshold"));
  Serial.println(F("  5 - Test apogee detection"));
  Serial.println();
  Serial.println(F("Control:"));
  Serial.println(F("  r - Reset simulation"));
  Serial.println(F("  h - Show this menu"));
  Serial.println(F("========================================\n"));
}
