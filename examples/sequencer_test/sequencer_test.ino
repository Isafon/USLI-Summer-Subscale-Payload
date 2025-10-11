/*
 * Sequencer Test with Simulated Flight
 *
 * This sketch tests the entire sequencer state machine by simulating
 * sensor inputs for a complete mission profile.
 *
 * Tests all 17 sequencer states:
 * - SBIT 0-5: Startup sequence
 * - LBIT 6-9: Launch sequence
 * - DBIT 10-15: Deploy sequence
 * - ABIT 16: Abort/shutdown
 *
 * Test Modes (via Serial Commands):
 * - '1' = Auto-run full mission simulation
 * - '2' = Step through states manually
 * - '3' = Test emergency abort
 * - '4' = Skip to specific state
 * - 's' = Advance to next state
 * - 'r' = Reset sequencer
 * - 'h' = Show help
 *
 * Hardware:
 * - Pin 13: Status LED (shows current phase by blink rate)
 * - Pin A1: Emergency abort button (optional)
 * - LEDs on pyro pins to visualize deployments (optional)
 */

// Sequencer state definitions (from sequencer.h)
enum SequencerState {
  SBIT_INIT_SEQ_IMU = 0,
  SBIT_STARTUP_BATTERY = 1,
  SBIT_STARTUP_TELEM = 2,
  SBIT_STARTUP_PAYLOAD = 3,
  SBIT_PAYLOAD_TELEM = 4,
  SBIT_RBSAFE_CHECK = 5,
  LBIT_IGNIT_BOOSTER = 6,
  LBIT_LAUNCH = 7,
  LBIT_POST_LAUNCH_REPORT = 8,
  LBIT_APOGEE_REPORT = 9,
  DBIT_POP_NOSE_FAIRING = 10,
  DBIT_STAGE_SEPARATION = 11,
  DBIT_BOOM_PAYLOAD = 12,
  DBIT_FINAL_MODE = 13,
  DBIT_DEPLOY_PARACHUTE = 14,
  DBIT_SEND_ALL_TELEM = 15,
  ABIT_KILL_ALL_PROCESSES = 16
};

enum MissionPhase {
  PHASE_STARTUP,
  PHASE_PREFLIGHT,
  PHASE_LAUNCH,
  PHASE_FLIGHT,
  PHASE_DEPLOY,
  PHASE_RECOVERY,
  PHASE_ABORT
};

// Flight simulation data
struct SimulatedSensors {
  float altitude;
  float velocity;
  float acceleration;
  float accel_magnitude;  // G-forces
  float temperature;
  float pressure;
  bool batteryOK;
  bool sensorsOK;
  bool payloadOK;
  float time;  // Mission elapsed time
} sim;

// Sequencer state
struct SequencerControl {
  SequencerState currentState;
  MissionPhase currentPhase;
  unsigned long stateStartTime;
  bool sequenceActive;
  bool emergencyAbort;
  float launchAltitude;
  float maxAltitude;
  bool apogeeDetected;
} sequencer;

// Test mode
enum TestMode {
  MODE_IDLE,
  MODE_AUTO,
  MODE_MANUAL
} testMode;

// Pin definitions
#define STATUS_LED_PIN 13
#define EMERGENCY_ABORT_PIN A1
#define PYRO_PIN_1 2
#define PYRO_PIN_2 4

// Thresholds
#define LAUNCH_ACCEL_THRESHOLD_G 2.0
#define APOGEE_VELOCITY_THRESHOLD -2.0
#define LANDING_VELOCITY_THRESHOLD 5.0

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(EMERGENCY_ABORT_PIN, INPUT_PULLUP);
  pinMode(PYRO_PIN_1, OUTPUT);
  pinMode(PYRO_PIN_2, OUTPUT);

  Serial.println(F("========================================"));
  Serial.println(F("Sequencer Test - Simulated Flight"));
  Serial.println(F("========================================\n"));

  Serial.println(F("This test simulates a complete mission"));
  Serial.println(F("and validates sequencer state transitions.\n"));

  initSequencer();
  initSimulation();
  printMenu();

  testMode = MODE_IDLE;
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  // Check emergency abort button
  if (digitalRead(EMERGENCY_ABORT_PIN) == LOW) {
    triggerEmergencyAbort();
    delay(500);  // Debounce
  }

  // Update based on test mode
  if (testMode == MODE_AUTO) {
    if (sequencer.sequenceActive) {
      updateSimulation();
      updateSequencer();
      delay(500);  // Slow down for readability
    } else {
      Serial.println(F("\n✓ Mission simulation complete!"));
      printMissionSummary();
      testMode = MODE_IDLE;
    }
  }

  // Update status LED
  blinkSequencerStatus();
}

void handleCommand(char cmd) {
  Serial.println();

  switch (cmd) {
    case '1':
      Serial.println(F("Starting automatic mission simulation..."));
      testMode = MODE_AUTO;
      initSequencer();
      initSimulation();
      break;

    case '2':
      Serial.println(F("Entering manual step mode"));
      Serial.println(F("Press 's' to advance to next state"));
      testMode = MODE_MANUAL;
      initSequencer();
      initSimulation();
      break;

    case '3':
      triggerEmergencyAbort();
      break;

    case '4':
      jumpToState();
      break;

    case 's':
    case 'S':
      if (testMode == MODE_MANUAL) {
        advanceState();
      } else {
        Serial.println(F("Enter manual mode first (press '2')"));
      }
      break;

    case 'r':
    case 'R':
      Serial.println(F("Resetting sequencer..."));
      initSequencer();
      initSimulation();
      testMode = MODE_IDLE;
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

void initSequencer() {
  sequencer.currentState = SBIT_INIT_SEQ_IMU;
  sequencer.currentPhase = PHASE_STARTUP;
  sequencer.stateStartTime = millis();
  sequencer.sequenceActive = true;
  sequencer.emergencyAbort = false;
  sequencer.launchAltitude = 0.0;
  sequencer.maxAltitude = 0.0;
  sequencer.apogeeDetected = false;

  Serial.println(F("\n✓ Sequencer initialized"));
  Serial.println(F("  State: SBIT-0 (Init Seq/IMU)"));
  Serial.println(F("  Phase: STARTUP\n"));
}

void initSimulation() {
  sim.altitude = 0;
  sim.velocity = 0;
  sim.acceleration = 0;
  sim.accel_magnitude = 1.0;
  sim.temperature = 22.0;
  sim.pressure = 1013.25;
  sim.batteryOK = true;
  sim.sensorsOK = true;
  sim.payloadOK = true;
  sim.time = 0;

  Serial.println(F("✓ Simulation initialized"));
  Serial.println(F("  All systems nominal\n"));
}

void updateSimulation() {
  sim.time += 0.5;  // Advance 0.5 seconds

  // Simulate flight profile based on current state
  switch (sequencer.currentState) {
    case LBIT_IGNIT_BOOSTER:
      // On pad, waiting for launch
      sim.accel_magnitude = 1.0;
      if (sim.time > 10.0) {
        // Trigger launch after 10 seconds
        sim.accel_magnitude = 8.0;
        sim.acceleration = 70.0;
        sim.velocity = 0;
      }
      break;

    case LBIT_LAUNCH:
    case LBIT_POST_LAUNCH_REPORT:
      // Powered ascent
      sim.accel_magnitude = 8.0;
      sim.acceleration = 70.0;
      sim.velocity += sim.acceleration * 0.5;
      sim.altitude += sim.velocity * 0.5;

      if (sim.altitude > 100.0) {
        // Motor burnout
        sim.accel_magnitude = 1.0;
        sim.acceleration = -9.81;
      }

      if (sim.altitude > sequencer.maxAltitude) {
        sequencer.maxAltitude = sim.altitude;
      }
      break;

    case LBIT_APOGEE_REPORT:
      // Coasting to apogee
      sim.accel_magnitude = 1.0;
      sim.acceleration = -9.81;
      sim.velocity += sim.acceleration * 0.5;
      if (sim.velocity < 0) sim.velocity = -5.0;  // Slow descent
      sim.altitude += sim.velocity * 0.5;
      break;

    case DBIT_POP_NOSE_FAIRING:
    case DBIT_STAGE_SEPARATION:
    case DBIT_BOOM_PAYLOAD:
    case DBIT_FINAL_MODE:
    case DBIT_DEPLOY_PARACHUTE:
    case DBIT_SEND_ALL_TELEM:
      // Descending with parachute
      sim.velocity = -10.0;  // Terminal velocity
      sim.altitude += sim.velocity * 0.5;
      if (sim.altitude < 0) sim.altitude = 0;
      sim.accel_magnitude = 1.0;
      break;
  }

  // Update temperature and pressure based on altitude
  sim.temperature = 22.0 - (sim.altitude * 0.0065);  // Standard lapse rate
  sim.pressure = 1013.25 * pow(1 - (0.0065 * sim.altitude / 288.15), 5.255);
}

void updateSequencer() {
  if (!sequencer.sequenceActive || sequencer.emergencyAbort) {
    return;
  }

  executeCurrentState();
}

void executeCurrentState() {
  Serial.println(F("\n----------------------------------------"));
  Serial.print(F("State: "));
  Serial.println(getStateString(sequencer.currentState));
  Serial.print(F("Phase: "));
  Serial.println(getPhaseString(sequencer.currentPhase));
  Serial.print(F("Time: "));
  Serial.print(sim.time, 1);
  Serial.println(F(" s"));
  printSimData();
  Serial.println(F("----------------------------------------"));

  switch (sequencer.currentState) {
    case SBIT_INIT_SEQ_IMU:
      Serial.println(F("▶ Initializing sequencer & IMU..."));
      if (sim.sensorsOK) {
        transitionToState(SBIT_STARTUP_BATTERY);
      }
      break;

    case SBIT_STARTUP_BATTERY:
      Serial.println(F("▶ Checking startup battery..."));
      if (sim.batteryOK) {
        transitionToState(SBIT_STARTUP_TELEM);
      }
      break;

    case SBIT_STARTUP_TELEM:
      Serial.println(F("▶ Starting telemetry systems..."));
      if (sim.sensorsOK) {
        transitionToState(SBIT_STARTUP_PAYLOAD);
      }
      break;

    case SBIT_STARTUP_PAYLOAD:
      Serial.println(F("▶ Checking payload battery..."));
      if (sim.payloadOK) {
        transitionToState(SBIT_PAYLOAD_TELEM);
      }
      break;

    case SBIT_PAYLOAD_TELEM:
      Serial.println(F("▶ Starting payload telemetry..."));
      if (sim.sensorsOK && sim.payloadOK) {
        transitionToState(SBIT_RBSAFE_CHECK);
      }
      break;

    case SBIT_RBSAFE_CHECK:
      Serial.println(F("▶ RBSAFE validation check..."));
      Serial.println(F("✓ SBIT sequence complete!"));
      transitionToState(LBIT_IGNIT_BOOSTER);
      transitionToPhase(PHASE_PREFLIGHT);
      break;

    case LBIT_IGNIT_BOOSTER:
      Serial.println(F("▶ Waiting for launch detection..."));
      if (sim.accel_magnitude > LAUNCH_ACCEL_THRESHOLD_G) {
        Serial.println(F("🚀 LAUNCH DETECTED!"));
        digitalWrite(PYRO_PIN_1, HIGH);
        delay(100);
        digitalWrite(PYRO_PIN_1, LOW);
        sequencer.launchAltitude = sim.altitude;
        transitionToState(LBIT_LAUNCH);
        transitionToPhase(PHASE_LAUNCH);
      }
      break;

    case LBIT_LAUNCH:
      Serial.println(F("▶ Launch confirmed!"));
      if (sim.altitude > sequencer.launchAltitude + 50.0) {
        transitionToState(LBIT_POST_LAUNCH_REPORT);
        transitionToPhase(PHASE_FLIGHT);
      }
      break;

    case LBIT_POST_LAUNCH_REPORT:
      Serial.println(F("▶ Tracking to apogee..."));
      if (sim.velocity < APOGEE_VELOCITY_THRESHOLD) {
        sequencer.apogeeDetected = true;
        transitionToState(LBIT_APOGEE_REPORT);
      }
      break;

    case LBIT_APOGEE_REPORT:
      Serial.println(F("⛰️  APOGEE DETECTED!"));
      Serial.print(F("  Max Altitude: "));
      Serial.print(sequencer.maxAltitude, 1);
      Serial.println(F(" m"));
      transitionToState(DBIT_POP_NOSE_FAIRING);
      transitionToPhase(PHASE_DEPLOY);
      break;

    case DBIT_POP_NOSE_FAIRING:
      Serial.println(F("▶ Popping nose fairing..."));
      digitalWrite(PYRO_PIN_1, HIGH);
      delay(100);
      digitalWrite(PYRO_PIN_1, LOW);
      transitionToState(DBIT_STAGE_SEPARATION);
      break;

    case DBIT_STAGE_SEPARATION:
      Serial.println(F("▶ Stage separation..."));
      digitalWrite(PYRO_PIN_2, HIGH);
      delay(100);
      digitalWrite(PYRO_PIN_2, LOW);
      transitionToState(DBIT_BOOM_PAYLOAD);
      break;

    case DBIT_BOOM_PAYLOAD:
      Serial.println(F("▶ Deploying payload..."));
      transitionToState(DBIT_FINAL_MODE);
      break;

    case DBIT_FINAL_MODE:
      Serial.println(F("▶ Final deploy mode..."));
      transitionToState(DBIT_DEPLOY_PARACHUTE);
      break;

    case DBIT_DEPLOY_PARACHUTE:
      Serial.println(F("🪂 Deploying parachute..."));
      transitionToState(DBIT_SEND_ALL_TELEM);
      transitionToPhase(PHASE_RECOVERY);
      break;

    case DBIT_SEND_ALL_TELEM:
      Serial.println(F("▶ Sending telemetry..."));
      if (sim.altitude <= 0 && abs(sim.velocity) < LANDING_VELOCITY_THRESHOLD) {
        Serial.println(F("🪂 LANDING DETECTED!"));
        transitionToState(ABIT_KILL_ALL_PROCESSES);
      }
      break;

    case ABIT_KILL_ALL_PROCESSES:
      Serial.println(F("▶ Mission complete - shutting down"));
      sequencer.sequenceActive = false;
      break;
  }
}

void transitionToState(SequencerState newState) {
  Serial.print(F("\n→ Transitioning: "));
  Serial.print(getStateString(sequencer.currentState));
  Serial.print(F(" -> "));
  Serial.println(getStateString(newState));

  sequencer.currentState = newState;
  sequencer.stateStartTime = millis();
}

void transitionToPhase(MissionPhase newPhase) {
  Serial.print(F("→ Phase change: "));
  Serial.print(getPhaseString(sequencer.currentPhase));
  Serial.print(F(" -> "));
  Serial.println(getPhaseString(newPhase));

  sequencer.currentPhase = newPhase;
}

void advanceState() {
  if (sequencer.currentState < ABIT_KILL_ALL_PROCESSES) {
    SequencerState nextState = (SequencerState)(sequencer.currentState + 1);
    transitionToState(nextState);
    Serial.println(F("Press 's' for next state"));
  } else {
    Serial.println(F("Already at final state"));
  }
}

void jumpToState() {
  Serial.println(F("\nEnter state number (0-16):"));
  Serial.println(F("0-5: SBIT, 6-9: LBIT, 10-15: DBIT, 16: ABIT"));
  // This is a simplified version - full implementation would wait for input
}

void triggerEmergencyAbort() {
  Serial.println(F("\n⚠️  EMERGENCY ABORT TRIGGERED! ⚠️"));
  sequencer.emergencyAbort = true;
  sequencer.sequenceActive = false;
  digitalWrite(PYRO_PIN_1, LOW);
  digitalWrite(PYRO_PIN_2, LOW);
  Serial.println(F("✓ All systems SAFE"));
}

void blinkSequencerStatus() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;

  unsigned long blinkRate = 1000;

  switch (sequencer.currentPhase) {
    case PHASE_STARTUP:     blinkRate = 2000; break;
    case PHASE_PREFLIGHT:   blinkRate = 500; break;
    case PHASE_LAUNCH:
    case PHASE_FLIGHT:      blinkRate = 100; break;
    case PHASE_DEPLOY:      blinkRate = 200; break;
    case PHASE_RECOVERY:    blinkRate = 1000; break;
    case PHASE_ABORT:       blinkRate = 50; break;
  }

  if (millis() - lastBlink >= blinkRate) {
    ledState = !ledState;
    digitalWrite(STATUS_LED_PIN, ledState);
    lastBlink = millis();
  }
}

void printSimData() {
  Serial.print(F("  Alt: "));
  Serial.print(sim.altitude, 1);
  Serial.print(F("m, Vel: "));
  Serial.print(sim.velocity, 1);
  Serial.print(F("m/s, Accel: "));
  Serial.print(sim.accel_magnitude, 1);
  Serial.println(F("G"));
}

void printMissionSummary() {
  Serial.println(F("\n========================================"));
  Serial.println(F("Mission Summary"));
  Serial.println(F("========================================"));
  Serial.print(F("Total Time: "));
  Serial.print(sim.time, 1);
  Serial.println(F(" s"));
  Serial.print(F("Max Altitude: "));
  Serial.print(sequencer.maxAltitude, 1);
  Serial.println(F(" m"));
  Serial.print(F("Final State: "));
  Serial.println(getStateString(sequencer.currentState));
  Serial.println(F("========================================\n"));
}

void printMenu() {
  Serial.println(F("========================================"));
  Serial.println(F("Command Menu"));
  Serial.println(F("========================================"));
  Serial.println(F("1 - Auto-run full mission"));
  Serial.println(F("2 - Manual step mode"));
  Serial.println(F("3 - Test emergency abort"));
  Serial.println(F("4 - Jump to specific state"));
  Serial.println(F("s - Advance state (manual mode)"));
  Serial.println(F("r - Reset sequencer"));
  Serial.println(F("h - Show help"));
  Serial.println(F("========================================\n"));
}

String getStateString(SequencerState state) {
  switch (state) {
    case SBIT_INIT_SEQ_IMU: return F("SBIT-0: Init Seq/IMU");
    case SBIT_STARTUP_BATTERY: return F("SBIT-1: Startup Battery");
    case SBIT_STARTUP_TELEM: return F("SBIT-2: Startup Telemetry");
    case SBIT_STARTUP_PAYLOAD: return F("SBIT-3: Startup Payload");
    case SBIT_PAYLOAD_TELEM: return F("SBIT-4: Payload Telemetry");
    case SBIT_RBSAFE_CHECK: return F("SBIT-5: RBSAFE Check");
    case LBIT_IGNIT_BOOSTER: return F("LBIT-6: Ignit Booster");
    case LBIT_LAUNCH: return F("LBIT-7: Launch");
    case LBIT_POST_LAUNCH_REPORT: return F("LBIT-8: Post Launch");
    case LBIT_APOGEE_REPORT: return F("LBIT-9: Apogee Report");
    case DBIT_POP_NOSE_FAIRING: return F("DBIT-10: Pop Nose Fairing");
    case DBIT_STAGE_SEPARATION: return F("DBIT-11: Stage Separation");
    case DBIT_BOOM_PAYLOAD: return F("DBIT-12: Boom Payload");
    case DBIT_FINAL_MODE: return F("DBIT-13: Final Mode");
    case DBIT_DEPLOY_PARACHUTE: return F("DBIT-14: Deploy Parachute");
    case DBIT_SEND_ALL_TELEM: return F("DBIT-15: Send All Telemetry");
    case ABIT_KILL_ALL_PROCESSES: return F("ABIT-16: Kill All Processes");
    default: return F("UNKNOWN");
  }
}

String getPhaseString(MissionPhase phase) {
  switch (phase) {
    case PHASE_STARTUP: return F("STARTUP");
    case PHASE_PREFLIGHT: return F("PREFLIGHT");
    case PHASE_LAUNCH: return F("LAUNCH");
    case PHASE_FLIGHT: return F("FLIGHT");
    case PHASE_DEPLOY: return F("DEPLOY");
    case PHASE_RECOVERY: return F("RECOVERY");
    case PHASE_ABORT: return F("ABORT");
    default: return F("UNKNOWN");
  }
}
