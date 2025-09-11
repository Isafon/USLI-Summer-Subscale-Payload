/*
 * SD Card Test - Slow SPI Speed
 */

#include <SD.h>
#include <SPI.h>

// SD Card CS pin
#define SD_CS_PIN 9

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== SD Card Slow Speed Test ==="));
  
  // Set SPI to slower speed
  SPI.setClockDivider(SPI_CLOCK_DIV128); // Very slow
  
  Serial.println(F("Testing SD.begin() with slow SPI..."));
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("  SD.begin() failed with slow SPI"));
    
    // Try with default speed
    Serial.println(F("Trying with default SPI speed..."));
    SPI.setClockDivider(SPI_CLOCK_DIV4); // Default speed
    if (!SD.begin(SD_CS_PIN)) {
      Serial.println(F("  SD.begin() failed with default speed too"));
    } else {
      Serial.println(F(" SD.begin() worked with default speed!"));
    }
  } else {
    Serial.println(F(" SD.begin() successful with slow SPI!"));
  }
  
  Serial.println(F("=== Test Complete ==="));
}

void loop() {
  // Blink LED to show system is alive
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}
