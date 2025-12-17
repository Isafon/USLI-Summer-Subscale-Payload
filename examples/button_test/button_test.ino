/*
 * Button Test - Standalone Button Diagnostics
 * 
 * This script tests the button functionality in isolation to verify:
 * - Button hardware connections
 * - Pin readings (raw digital state)
 * - Button press detection
 * - Debouncing behavior
 * 
 * Button Configuration:
 * - Pin: 4 (D4)
 * - Mode: INPUT_PULLUP (internal pull-up resistor)
 * - Expected: HIGH when not pressed, LOW when pressed (connected to GND)
 * 
 * Wiring:
 * - Button one side -> Pin 4
 * - Button other side -> GND
 * - Internal pull-up keeps pin HIGH when button is open
 * - Pressing button connects pin to GND, making it LOW
 */

#define BUTTON_PIN 4       // Same pin as in main code
#define LED_PIN 13         // Built-in LED for visual feedback

// Button state tracking
bool lastButtonState = HIGH;
unsigned long lastButtonPress = 0;
unsigned long lastStateChange = 0;
unsigned long debounceDelay = 100;  // Same as main code

// Statistics
unsigned long pressCount = 0;
unsigned long lastPressTime = 0;
unsigned long lastReleaseTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);  // Wait for serial port to connect
  }
  
  delay(500);  // Give time for serial monitor to connect
  
  Serial.println(F("\n========================================"));
  Serial.println(F("     BUTTON TEST - Standalone"));
  Serial.println(F("========================================"));
  Serial.println(F(""));
  Serial.print(F("Testing button on Pin: "));
  Serial.println(BUTTON_PIN);
  Serial.print(F("Mode: INPUT_PULLUP"));
  Serial.println(F(""));
  Serial.println(F("Expected behavior:"));
  Serial.println(F("  - HIGH (1) when button NOT pressed"));
  Serial.println(F("  - LOW (0) when button PRESSED"));
  Serial.println(F(""));
  Serial.println(F("========================================"));
  Serial.println(F("Starting continuous monitoring..."));
  Serial.println(F("Press button to test"));
  Serial.println(F("========================================"));
  Serial.println(F(""));
  
  // Initialize button pin with internal pull-up (same as main code)
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize LED for visual feedback
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Read initial state
  lastButtonState = digitalRead(BUTTON_PIN);
  
  Serial.print(F("Initial button state: "));
  Serial.println(lastButtonState == HIGH ? F("HIGH (NOT PRESSED)") : F("LOW (PRESSED)"));
  Serial.println(F(""));
  
  // Print header for continuous readings
  Serial.println(F("Format: [Time ms] Raw State | Change | Debounced | Stats"));
  Serial.println(F("----------------------------------------------------------"));
}

void loop() {
  unsigned long currentTime = millis();
  
  // Read current button state (raw, no debouncing)
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  // Determine state as string
  const char* stateStr = currentButtonState == HIGH ? "HIGH" : "LOW ";
  const char* stateDesc = currentButtonState == HIGH ? "NOT PRESSED" : "PRESSED  ";
  
  // Check for state change
  bool stateChanged = (currentButtonState != lastButtonState);
  
  // Update LED to match button state (inverted because pull-up)
  digitalWrite(LED_PIN, currentButtonState == LOW ? HIGH : LOW);
  
  // Print continuous reading every 200ms or on state change
  static unsigned long lastPrintTime = 0;
  bool shouldPrint = false;
  
  if (stateChanged) {
    shouldPrint = true;
    lastStateChange = currentTime;
  } else if (currentTime - lastPrintTime >= 500) {
    // Print status every 500ms even if no change
    shouldPrint = true;
  }
  
  if (shouldPrint) {
    // Print timestamp and raw state
    Serial.print(F("["));
    Serial.print(currentTime);
    Serial.print(F("] "));
    Serial.print(stateStr);
    Serial.print(F(" ("));
    Serial.print(currentButtonState);
    Serial.print(F(") = "));
    Serial.print(stateDesc);
    
    // Print state change indicator
    if (stateChanged) {
      Serial.print(F(" | *** STATE CHANGED ***"));
    } else {
      Serial.print(F(" |          "));
    }
    
    // Check for debounced press (same logic as main code)
    bool debouncedPress = false;
    if (currentButtonState == LOW && lastButtonState == HIGH) {
      // Button was just pressed
      if (currentTime - lastButtonPress > debounceDelay) {
        debouncedPress = true;
        pressCount++;
        lastPressTime = currentTime;
        lastButtonPress = currentTime;
      }
    } else if (currentButtonState == HIGH && lastButtonState == LOW) {
      // Button was just released
      lastReleaseTime = currentTime;
    }
    
    if (debouncedPress) {
      Serial.print(F(" | DEBOUNCED PRESS #"));
      Serial.print(pressCount);
    } else {
      Serial.print(F(" |             "));
    }
    
    // Print statistics
    Serial.print(F(" | Presses: "));
    Serial.print(pressCount);
    
    if (lastPressTime > 0) {
      Serial.print(F(", Last press: "));
      Serial.print(currentTime - lastPressTime);
      Serial.print(F("ms ago"));
    }
    
    // Show time since last state change if recent
    if (stateChanged) {
      Serial.print(F(" [CHANGE]"));
    }
    
    Serial.println();
    
    lastPrintTime = currentTime;
  }
  
  // Update last state
  lastButtonState = currentButtonState;
  
  // Small delay to prevent overwhelming serial output
  delay(10);
}


