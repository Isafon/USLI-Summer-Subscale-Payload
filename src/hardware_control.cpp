#include "hardware_control.h"
#include "sequencer.h"
#include "temp.h"
#include "gps.h"
#include "config.h"

void initHardware() {
  Serial.println(F("Initializing hardware control pins..."));
  
  // Initialize hardware control pins
  pinMode(BOOSTER_IGNITION_PIN, OUTPUT);
  pinMode(NOSE_FAIRING_PIN, OUTPUT);
  pinMode(STAGE_SEPARATION_PIN, OUTPUT);
  pinMode(PAYLOAD_DEPLOY_PIN, OUTPUT);
  pinMode(PARACHUTE_DEPLOY_PIN, OUTPUT);
  pinMode(EMERGENCY_ABORT_PIN, INPUT_PULLUP);
  pinMode(BATTERY_MONITOR_PIN, INPUT);
  pinMode(PAYLOAD_POWER_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Ensure all deployment systems are safe
  safeAllPyrotechnics();
  
  // Power on payload
  enablePayloadPower();
  
  Serial.println(F("Hardware initialization complete"));
}

void igniteBooster() {
  Serial.println(F(">>> BOOSTER IGNITION <<<"));
  digitalWrite(BOOSTER_IGNITION_PIN, HIGH);
  delay(100); // Brief pulse
  digitalWrite(BOOSTER_IGNITION_PIN, LOW);
}

void popNoseFairing() {
  Serial.println(F(">>> NOSE FAIRING DEPLOYMENT <<<"));
  digitalWrite(NOSE_FAIRING_PIN, HIGH);
  delay(100);
  digitalWrite(NOSE_FAIRING_PIN, LOW);
}

void separateStage() {
  Serial.println(F(">>> STAGE SEPARATION <<<"));
  digitalWrite(STAGE_SEPARATION_PIN, HIGH);
  delay(100);
  digitalWrite(STAGE_SEPARATION_PIN, LOW);
}

void deployPayload() {
  Serial.println(F(">>> PAYLOAD DEPLOYMENT <<<"));
  digitalWrite(PAYLOAD_DEPLOY_PIN, HIGH);
  delay(100);
  digitalWrite(PAYLOAD_DEPLOY_PIN, LOW);
}

void deployParachute() {
  Serial.println(F(">>> PARACHUTE DEPLOYMENT <<<"));
  digitalWrite(PARACHUTE_DEPLOY_PIN, HIGH);
  delay(100);
  digitalWrite(PARACHUTE_DEPLOY_PIN, LOW);
}

void safeAllPyrotechnics() {
  Serial.println(F("Safing all pyrotechnic systems"));
  digitalWrite(BOOSTER_IGNITION_PIN, LOW);
  digitalWrite(NOSE_FAIRING_PIN, LOW);
  digitalWrite(STAGE_SEPARATION_PIN, LOW);
  digitalWrite(PAYLOAD_DEPLOY_PIN, LOW);
  digitalWrite(PARACHUTE_DEPLOY_PIN, LOW);
}

void enablePayloadPower() {
  digitalWrite(PAYLOAD_POWER_PIN, HIGH);
  Serial.println(F("Payload power enabled"));
}

void disablePayloadPower() {
  digitalWrite(PAYLOAD_POWER_PIN, LOW);
  Serial.println(F("Payload power disabled"));
}

void checkBatteryStatus() {
  int batteryReading = analogRead(BATTERY_MONITOR_PIN);
  float batteryVoltage = (batteryReading / 1023.0) * 5.0; // Assuming 5V reference
  sequencerData.batteryOK = (batteryVoltage > 3.3); // Minimum acceptable voltage
  
  if (!sequencerData.batteryOK) {
    Serial.print(F("WARNING: Low battery voltage: "));
    Serial.print(batteryVoltage);
    Serial.println(F("V"));
  }
}

bool checkSensorStatus() {
  // Check if core sensors are responding
  sequencerData.sensorsOK = true;
  
  // Temperature sensor check
  float temp = readTemperatureC();
  if (temp == -999.0) {
    Serial.println(F("ERROR: Temperature sensor failure"));
    sequencerData.sensorsOK = false;
  }
  
  // GPS check
  if (gps.satellites.value() == 0 && millis() > 60000) {
    Serial.println(F("WARNING: No GPS satellites after 60 seconds"));
    // Don't fail for GPS - it's not critical for flight safety
  }
  
  return sequencerData.sensorsOK;
}