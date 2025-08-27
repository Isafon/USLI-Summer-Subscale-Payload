/*
 * USLI Payload - LED Test Sketch
 * Simple test to verify Arduino is working
 */

// LED pin (built-in LED on Arduino Nano)
#define LED_PIN 13

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println(F("=== USLI Payload LED Test ==="));
  Serial.println(F("Arduino is working!"));
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println(F("LED will blink every second"));
  Serial.println(F("Press Ctrl+C to stop monitoring"));
}

void loop() {
  // Turn LED on
  digitalWrite(LED_PIN, HIGH);
  Serial.println(F("LED ON"));
  delay(1000);
  
  // Turn LED off
  digitalWrite(LED_PIN, LOW);
  Serial.println(F("LED OFF"));
  delay(1000);
}
