#ifndef HARDWARE_CONTROL_H
#define HARDWARE_CONTROL_H

#include <Arduino.h>

// Hardware control pins
#define BOOSTER_IGNITION_PIN 2
#define NOSE_FAIRING_PIN 4
#define STAGE_SEPARATION_PIN 5
#define PAYLOAD_DEPLOY_PIN 7
#define PARACHUTE_DEPLOY_PIN A0
#define EMERGENCY_ABORT_PIN A1
#define BATTERY_MONITOR_PIN A2
#define PAYLOAD_POWER_PIN A3

// Status indicators
#define STATUS_LED_PIN 13
#define BUZZER_PIN A4

// Hardware control functions
void initHardware();
void igniteBooster();
void popNoseFairing();
void separateStage();
void deployPayload();
void deployParachute();
void safeAllPyrotechnics();
void enablePayloadPower();
void disablePayloadPower();
void checkBatteryStatus();
bool checkSensorStatus();

#endif // HARDWARE_CONTROL_H