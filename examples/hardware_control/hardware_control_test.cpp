/*
 * Hardware Control Test
 *
 * This sketch tests all hardware control outputs (pyrotechnic channels,
 * status LEDs, etc.) using LEDs instead of actual pyrotechnics.
 *
 * SAFETY WARNING: NEVER connect actual pyrotechnics during testing!
 * Always use LEDs or buzzers for ground testing.
 *
 * Hardware Connections (using LEDs for safety):
 * - Pin 2  (Booster Ignition)   -> LED + 220Ω resistor -> GND
 * - Pin 4  (Nose Fairing)        -> LED + 220Ω resistor -> GND
 * - Pin 5  (Stage Separation)    -> LED + 220Ω resistor -> GND
 * - Pin 7  (Payload Deploy)      -> LED + 220Ω resistor -> GND
 * - Pin A0 (Parachute Deploy)    -> LED + 220Ω resistor -> GND
 * - Pin A1 (Emergency Abort)     -> Button to GND (INPUT_PULLUP)
 * - Pin A2 (Battery Monitor)     -> Voltage divider (optional)
 * - Pin A3 (Payload Power)       -> LED + 220Ω resistor -> GND
 * - Pin 13 (Status LED)          -> Built-in LED
 * - Pin A4 (Buzzer)              -> Buzzer (optional)
 *
 * Test Modes (via Serial Commands):
 * - '1' = Test booster ignition
 * - '2' = Test nose fairing
 * - '3' = Test stage separation
 * - '4' = Test payload deploy
 * - '5' = Test parachute deploy
 * - 'a' = Test all channels sequentially
 * - 'b' = Test battery monitor
 * - 'p' = Toggle payload power
 * - 's' = Safe all pyrotechnics
 * - 'h' = Show help menu
 */

#include <Arduino.h>

// Pin definitions (from hardware_control.h)
#define BOOSTER_IGNITION_PIN 2
#define NOSE_FAIRING_PIN 4
#define STAGE_SEPARATION_PIN 5
#define PAYLOAD_DEPLOY_PIN 7
#define PARACHUTE_DEPLOY_PIN A0
#define EMERGENCY_ABORT_PIN A1
#define BATTERY_MONITOR_PIN A2
#define PAYLOAD_POWER_PIN A3
#define STATUS_LED_PIN 13
#define BUZZER_PIN A4

// Pyrotechnic fire duration (milliseconds)
#define PYRO_FIRE_DURATION 1000  // 1 second for testing (real: 100-200ms)

// State tracking
bool payloadPowerEnabled = false;
unsigned long lastStatusBlink = 0;
bool statusLedState = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("========================================"));
  Serial.println(F("Hardware Control Test"));
  Serial.println(F("========================================"));
  Serial.println();
  Serial.println(F("⚠️  SAFETY WARNING ⚠️"));
  Serial.println(F("This test uses LEDs to simulate pyrotechnics."));
  Serial.println(F("NEVER connect actual pyrotechnics during testing!"));
  Serial.println(F("========================================\n"));

  // Initialize all output pins
  pinMode(BOOSTER_IGNITION_PIN, OUTPUT);
  pinMode(NOSE_FAIRING_PIN, OUTPUT);
  pinMode(STAGE_SEPARATION_PIN, OUTPUT);
  pinMode(PAYLOAD_DEPLOY_PIN, OUTPUT);
  pinMode(PARACHUTE_DEPLOY_PIN, OUTPUT);
  pinMode(PAYLOAD_POWER_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize input pins
  pinMode(EMERGENCY_ABORT_PIN, INPUT_PULLUP);

  // Safe all channels on startup
  safeAllPyrotechnics();
  digitalWrite(PAYLOAD_POWER_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println(F("✓ All pins initialized"));
  Serial.println(F("✓ All channels SAFE\n"));

  // Pin verification
  Serial.println(F("Pin Configuration:"));
  Serial.println(F("  Pyrotechnic Channels:"));
  Serial.println(F("    Pin 2  - Booster Ignition"));
  Serial.println(F("    Pin 4  - Nose Fairing"));
  Serial.println(F("    Pin 5  - Stage Separation"));
  Serial.println(F("    Pin 7  - Payload Deploy"));
  Serial.println(F("    Pin A0 - Parachute Deploy"));
  Serial.println(F("  Control Pins:"));
  Serial.println(F("    Pin A1 - Emergency Abort (button input)"));
  Serial.println(F("    Pin A2 - Battery Monitor (analog input)"));
  Serial.println(F("    Pin A3 - Payload Power Control"));
  Serial.println(F("  Status Indicators:"));
  Serial.println(F("    Pin 13 - Status LED"));
  Serial.println(F("    Pin A4 - Buzzer"));
  Serial.println();

  printMenu();

  // Startup beep sequence
  beep(100);
  delay(100);
  beep(100);
  Serial.println(F("✓ Ready for testing!\n"));
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  // Check emergency abort button
  if (digitalRead(EMERGENCY_ABORT_PIN) == LOW) {
    Serial.println(F("\n⚠️  EMERGENCY ABORT TRIGGERED! ⚠️"));
    safeAllPyrotechnics();
    digitalWrite(PAYLOAD_POWER_PIN, LOW);
    payloadPowerEnabled = false;

    // Rapid beeping
    for (int i = 0; i < 5; i++) {
      beep(100);
      delay(100);
    }

    Serial.println(F("✓ All systems SAFE\n"));
    delay(1000); // Debounce
  }

  // Blink status LED
  if (millis() - lastStatusBlink > 500) {
    statusLedState = !statusLedState;
    digitalWrite(STATUS_LED_PIN, statusLedState);
    lastStatusBlink = millis();
  }
}

void handleCommand(char cmd) {
  Serial.println();

  switch (cmd) {
    case '1':
      testChannel("BOOSTER IGNITION", BOOSTER_IGNITION_PIN);
      break;

    case '2':
      testChannel("NOSE FAIRING", NOSE_FAIRING_PIN);
      break;

    case '3':
      testChannel("STAGE SEPARATION", STAGE_SEPARATION_PIN);
      break;

    case '4':
      testChannel("PAYLOAD DEPLOY", PAYLOAD_DEPLOY_PIN);
      break;

    case '5':
      testChannel("PARACHUTE DEPLOY", PARACHUTE_DEPLOY_PIN);
      break;

    case 'a':
    case 'A':
      testAllSequence();
      break;

    case 'b':
    case 'B':
      testBatteryMonitor();
      break;

    case 'p':
    case 'P':
      togglePayloadPower();
      break;

    case 's':
    case 'S':
      safeAllPyrotechnics();
      Serial.println(F("✓ All pyrotechnic channels SAFE"));
      beep(200);
      break;

    case 'h':
    case 'H':
      printMenu();
      break;

    case '\n':
    case '\r':
      // Ignore newlines
      break;

    default:
      Serial.print(F("✗ Unknown command: "));
      Serial.println(cmd);
      Serial.println(F("Type 'h' for help"));
      break;
  }
}

void testChannel(const char* channelName, int pin) {
  Serial.print(F("🔥 Testing "));
  Serial.print(channelName);
  Serial.println(F("..."));

  Serial.print(F("   Pin "));
  Serial.print(pin);
  Serial.print(F(" -> HIGH for "));
  Serial.print(PYRO_FIRE_DURATION);
  Serial.println(F("ms"));

  // Fire the channel
  digitalWrite(pin, HIGH);
  beep(50);
  delay(PYRO_FIRE_DURATION);
  digitalWrite(pin, LOW);
  beep(50);

  Serial.print(F("✓ "));
  Serial.print(channelName);
  Serial.println(F(" test complete"));
  Serial.println();
}

void testAllSequence() {
  Serial.println(F("========================================"));
  Serial.println(F("Testing ALL channels sequentially"));
  Serial.println(F("========================================\n"));

  testChannel("BOOSTER IGNITION", BOOSTER_IGNITION_PIN);
  delay(500);

  testChannel("NOSE FAIRING", NOSE_FAIRING_PIN);
  delay(500);

  testChannel("STAGE SEPARATION", STAGE_SEPARATION_PIN);
  delay(500);

  testChannel("PAYLOAD DEPLOY", PAYLOAD_DEPLOY_PIN);
  delay(500);

  testChannel("PARACHUTE DEPLOY", PARACHUTE_DEPLOY_PIN);
  delay(500);

  Serial.println(F("========================================"));
  Serial.println(F("✓ All channel tests complete!"));
  Serial.println(F("========================================\n"));

  // Success beeps
  beep(100);
  delay(100);
  beep(100);
  delay(100);
  beep(100);
}

void testBatteryMonitor() {
  Serial.println(F("Testing Battery Monitor..."));
  Serial.println(F("Reading analog value from pin A2"));
  Serial.println();

  for (int i = 0; i < 10; i++) {
    int rawValue = analogRead(BATTERY_MONITOR_PIN);
    float voltage = rawValue * (5.0 / 1023.0); // Convert to voltage

    Serial.print(F("  Reading #"));
    Serial.print(i + 1);
    Serial.print(F(": Raw="));
    Serial.print(rawValue);
    Serial.print(F(", Voltage="));
    Serial.print(voltage, 3);
    Serial.println(F("V"));

    delay(200);
  }

  Serial.println(F("\n✓ Battery monitor test complete"));
  Serial.println(F("Note: If using voltage divider, apply formula:"));
  Serial.println(F("  Vbattery = Vmeasured * (R1 + R2) / R2\n"));
}

void togglePayloadPower() {
  payloadPowerEnabled = !payloadPowerEnabled;
  digitalWrite(PAYLOAD_POWER_PIN, payloadPowerEnabled ? HIGH : LOW);

  Serial.print(F("Payload Power: "));
  if (payloadPowerEnabled) {
    Serial.println(F("✓ ENABLED"));
    beep(100);
  } else {
    Serial.println(F("✗ DISABLED"));
    beep(200);
  }
  Serial.println();
}

void safeAllPyrotechnics() {
  digitalWrite(BOOSTER_IGNITION_PIN, LOW);
  digitalWrite(NOSE_FAIRING_PIN, LOW);
  digitalWrite(STAGE_SEPARATION_PIN, LOW);
  digitalWrite(PAYLOAD_DEPLOY_PIN, LOW);
  digitalWrite(PARACHUTE_DEPLOY_PIN, LOW);
}

void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void printMenu() {
  Serial.println(F("========================================"));
  Serial.println(F("Command Menu"));
  Serial.println(F("========================================"));
  Serial.println(F("Individual Channel Tests:"));
  Serial.println(F("  1 - Test Booster Ignition (Pin 2)"));
  Serial.println(F("  2 - Test Nose Fairing (Pin 4)"));
  Serial.println(F("  3 - Test Stage Separation (Pin 5)"));
  Serial.println(F("  4 - Test Payload Deploy (Pin 7)"));
  Serial.println(F("  5 - Test Parachute Deploy (Pin A0)"));
  Serial.println();
  Serial.println(F("System Tests:"));
  Serial.println(F("  a - Test ALL channels (sequential)"));
  Serial.println(F("  b - Test Battery Monitor"));
  Serial.println(F("  p - Toggle Payload Power"));
  Serial.println(F("  s - SAFE all channels"));
  Serial.println(F("  h - Show this menu"));
  Serial.println(F("========================================\n"));
}
