 #include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "baro_bmp280.h"
#include "rtc_pcf8523.h"
#include "uSD.h"

#define TEST_INTERVAL 500 // 1 second
#define BUTTON_PIN 4       // Button connected to pin 4

// Button state tracking
bool lastButtonState = HIGH;
unsigned long lastButtonPress = 0;
unsigned long debounceDelay = 100;   // 50ms debounce delay

// Component status tracking
bool rtcOK = false;
bool baroOK = false;
bool sdOK = false;

// Buzzer control
unsigned long lastBeepTime = 0;
unsigned long beepInterval = 1000;  // Beep every 2 seconds while recording
unsigned long beepStartTime = 0;
bool beeping = false;

// Flight events
float baseAlt = 0.0;
bool takeoff = false;
bool landing = false;

// Error buzzer
void errorBuzzer() {
  tone(BUZZER_PIN, 1000, 1500);
  delay(1600);
}

// Helper function to format timestamp
void formatTimestamp(char* buffer, size_t bufferSize, const DateTime& dt) {
  snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d", 
           dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
}

// Log system events to main data file
void logSystemEvent(const char* event, const char* message) {
  // Simplified - only log critical events
  if (sdOK && isLoggingActive() && rtcOK && strcmp(event, "ERROR") == 0) {
    DateTime dt;
    if (readRTC(dt)) {
      writeData(dt, event, message);
    }
  }
}


// Handle buzzer beeping while recording
void updateBuzzer() {
  if (isLoggingActive()) {
    // While recording, beep every beepInterval milliseconds
    if (!beeping && millis() - lastBeepTime >= beepInterval) {
      // Start a new chirp using tone()
      tone(BUZZER_PIN, 5000, 100);  // 2kHz tone for 50ms
      beepStartTime = millis();
      beeping = true;
    } else if (beeping && millis() - beepStartTime >= 100) {
      // End the beep after 100ms (tone() handles the duration)
      noTone(BUZZER_PIN);  // Stop any tone
      beeping = false;
      lastBeepTime = millis();
    }
  } else {
    // Not recording, ensure buzzer is off
    if (beeping) {
      noTone(BUZZER_PIN);
      beeping = false;
    }
  }
}

// Handle button press to toggle logging
void handleButtonPress() {
  Serial.println(F("Button pressed"));
  
  if (isLoggingActive()) {
    if (stopLogging()) {
      Serial.println(F("Logging stopped"));
    }
  } else {
    if (startLogging("data.csv")) {
      Serial.println(F("Logging started"));
      baseAlt = 0.0;
      takeoff = false;
      landing = false;
    }
  }
}

// Handle serial commands
void handleCommand(char cmd) {
  Serial.println();
  
  switch (cmd) {
    case 'l':
    case 'L':
      // Start logging
      if (isLoggingActive()) {
        Serial.println(F("Already logging to: "));
        Serial.println(getCurrentFileName());
      } else {
        Serial.println(F("Starting logging..."));
        if (startLogging("data.csv")) {
          Serial.println(F("Logging started"));
          baseAlt = 0.0;
          takeoff = false;
          landing = false;
        }
      }
      break;
      
    case 's':
    case 'S':
      // Stop logging
      if (isLoggingActive()) {
        Serial.println(F("Stopping logging..."));
        if (stopLogging()) {
          Serial.println(F("Logging stopped"));
        }
      } else {
        Serial.println(F("Not currently logging"));
      }
      break;
      
    case 'd':
    case 'D':
      // Delete current log file
      if (isLoggingActive()) {
        Serial.println(F("Stopping logging..."));
        stopLogging();
      }
      Serial.println(F("Deleting data.csv..."));
      if (deleteFile("data.csv")) {
        Serial.println(F("File deleted"));
      } else {
        Serial.println(F("File not found or delete failed"));
      }
      break;
      
    case 'h':
    case 'H':
      // Show help
      Serial.println(F("L=Start, S=Stop, D=Delete"));
      break;
      
    case '\n':
    case '\r':
      break;
      
    default:
      Serial.print(F("Unknown command: "));
      Serial.println(cmd);
      Serial.println(F("Press 'H' for help"));
      break;
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("USLI Payload"));

  // Initialize button pin with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println(F("✓ Button OK"));

  // Initialize buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Start with buzzer off
  Serial.println(F("✓ Buzzer OK"));

  // Initialize SPI bus once for all SPI devices
  SPI.begin();
  Serial.println(F("✓ SPI OK"));

  // Initialize RTC first
  rtcOK = initRTC();
  if (!rtcOK) {
    Serial.println(F("⚠ RTC failed"));
    errorBuzzer();
  } else {
    Serial.println(F("✓ RTC OK"));
  }
  
  // Check RTC time (but don't disable if invalid)
  if (rtcOK) {
    DateTime currentTime;
    if (readRTC(currentTime)) {
      Serial.print(F("RTC time: "));
      Serial.print(currentTime.year); Serial.print(F("-"));
      Serial.print(currentTime.month); Serial.print(F("-"));
      Serial.print(currentTime.day); Serial.print(F(" "));
      Serial.print(currentTime.hour); Serial.print(F(":"));
      Serial.print(currentTime.minute); Serial.print(F(":"));
      Serial.println(currentTime.second);
      
      // Warn if time seems invalid but don't disable
      if (currentTime.year < 2024) {
        Serial.println(F("WARNING: RTC time may be invalid"));
      }
    } else {
      Serial.println(F("WARNING: Cannot read RTC time"));
    }
  }

  // Initialize barometer
  baroOK = initBaro();
  if (!baroOK) {
    Serial.println(F("⚠ Barometer failed"));
    errorBuzzer();
  } else {
    Serial.println(F("✓ Barometer OK"));
  }

  // Initialize microSD card
  sdOK = initSD();
  if (!sdOK) {
    Serial.println(F("⚠ SD card failed"));
    errorBuzzer();
  } else {
    Serial.println(F("✓ SD card OK"));
  }

  // System startup complete

  Serial.println(F("\n=== STATUS ==="));
  Serial.print(F("RTC: ")); Serial.println(rtcOK ? F("OK") : F("FAILED"));
  Serial.print(F("Barometer: ")); Serial.println(baroOK ? F("OK") : F("FAILED"));
  Serial.print(F("SD Card: ")); Serial.println(sdOK ? F("OK") : F("FAILED"));
  Serial.println(F("================"));
  
  if (sdOK) {
    Serial.println(F("Ready"));
  } else {
    Serial.println(F("SD failed"));
  }

  Serial.println(F("\nL=Start, S=Stop, D=Delete"));
}

void loop() {
  // Read RTC data (if available)
  DateTime dt;
  char timestamp[32];
  
  if (rtcOK && readRTC(dt)) {
    formatTimestamp(timestamp, sizeof(timestamp), dt);
  } else {
    strcpy(timestamp, "NO-RTC");
  }
  
  // Read barometer data (if available)
  BaroData data;
  bool baroDataOK = false;
  
  if (baroOK && readBaro(data)) {
    baroDataOK = true;
  } else {
    // Set default values if barometer not available
    data.temperature = 0.0;
    data.pressure = 0.0;
    data.altitude = 0.0;
  }

  // Print sensor data only when logging is active
  if (isLoggingActive()) {
    if (baroDataOK) {
      Serial.print(timestamp);
      Serial.print(F(" "));
      Serial.print(data.temperature, 1);
      Serial.print(F("°C "));
      Serial.print(data.pressure, 1);
      Serial.print(F("hPa "));
      Serial.print(data.altitude, 1);
      Serial.println(F("m"));
      
      // Flight events
      if (isLoggingActive() && rtcOK) {
        if (baseAlt == 0.0 && data.altitude > 0) baseAlt = data.altitude;
        else if (!takeoff && data.altitude > baseAlt + 1.0) {
          takeoff = true;
          Serial.println(F("*** TAKEOFF DETECTED! ***"));
          DateTime dt;
          if (readRTC(dt)) writeData(dt, "TAKEOFF", "T");
        }
        else if (takeoff && !landing && data.altitude < baseAlt + 1.0) {
          landing = true;
          Serial.println(F("*** LANDING DETECTED! ***"));
          DateTime dt;
          if (readRTC(dt)) writeData(dt, "LANDING", "L");
        }
      }
    } else {
      Serial.println(F("NO-BARO"));
    }
  }

  // Write data to SD card (always try to write if SD available)
  if (sdOK && isLoggingActive()) {
    if (!writeData(dt, data)) {
      Serial.println(F("SD write failed"));
    }
  }

  // Update buzzer (beeps while recording)
  updateBuzzer();

  // Check for button press
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  // Check for button press (LOW when pressed, HIGH when not pressed)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    // Button was just pressed
    if (millis() - lastButtonPress > debounceDelay) {
      handleButtonPress();
      lastButtonPress = millis();
    }
  }
  
  lastButtonState = currentButtonState;

  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }

  delay(TEST_INTERVAL);
}
