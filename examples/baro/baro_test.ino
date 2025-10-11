/*
 * BMP280 Barometric Pressure Sensor Test
 *
 * This sketch tests the BMP280 barometer to verify:
 * - I2C communication is working
 * - Sensor initializes correctly
 * - Pressure readings are valid
 * - Temperature readings are valid
 * - Altitude calculation works
 *
 * Hardware Connections:
 * - VCC -> 3.3V or 5V (check your module)
 * - GND -> GND
 * - SDA -> A4 (Arduino Nano)
 * - SCL -> A5 (Arduino Nano)
 *
 * Expected Output:
 * - Pressure: ~1013 hPa (at sea level)
 * - Temperature: Room temperature (20-25°C)
 * - Altitude: ~0m (at sea level)
 */

#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C

// Test parameters
#define TEST_INTERVAL 1000  // Read sensor every 1 second
#define SEA_LEVEL_PRESSURE 1013.25  // Adjust for your location

// Statistics tracking
float minPressure = 999999;
float maxPressure = 0;
float minTemp = 999999;
float maxTemp = -999999;
float minAltitude = 999999;
float maxAltitude = -999999;
int readingCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // Wait for serial port

  Serial.println(F("========================================"));
  Serial.println(F("BMP280 Barometer Test"));
  Serial.println(F("========================================"));
  Serial.println();

  // Initialize I2C
  Wire.begin();

  // Test I2C connection
  Serial.println(F("Testing I2C bus..."));
  Wire.beginTransmission(0x76); // BMP280 default address
  byte error = Wire.endTransmission();

  if (error == 0) {
    Serial.println(F("✓ BMP280 detected at address 0x76"));
  } else {
    Wire.beginTransmission(0x77); // Alternative address
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.println(F("✓ BMP280 detected at address 0x77"));
    } else {
      Serial.println(F("✗ BMP280 NOT detected on I2C bus!"));
      Serial.println(F("  Check wiring:"));
      Serial.println(F("    - SDA -> A4"));
      Serial.println(F("    - SCL -> A5"));
      Serial.println(F("    - VCC -> 3.3V or 5V"));
      Serial.println(F("    - GND -> GND"));
      while (1) delay(1000); // Halt
    }
  }

  // Initialize sensor
  Serial.println(F("\nInitializing BMP280..."));
  if (!bmp.begin(0x76) && !bmp.begin(0x77)) {
    Serial.println(F("✗ Failed to initialize BMP280!"));
    Serial.println(F("  Could not find a valid BMP280 sensor"));
    while (1) delay(1000); // Halt
  }

  Serial.println(F("✓ BMP280 initialized successfully!"));

  // Configure sensor for high precision
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time

  Serial.println(F("\nSensor Configuration:"));
  Serial.println(F("  - Mode: NORMAL (continuous measurement)"));
  Serial.println(F("  - Temp Oversampling: x2"));
  Serial.println(F("  - Pressure Oversampling: x16 (high precision)"));
  Serial.println(F("  - Filter: x16"));
  Serial.println(F("  - Standby Time: 500ms"));

  Serial.println(F("\n========================================"));
  Serial.println(F("Starting continuous readings..."));
  Serial.println(F("========================================"));
  Serial.println(F("Time(ms)\tTemp(°C)\tPressure(hPa)\tAltitude(m)"));

  delay(1000); // Allow sensor to stabilize
}

void loop() {
  unsigned long startTime = millis();

  // Read sensor data
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  float altitude = bmp.readAltitude(SEA_LEVEL_PRESSURE);

  // Check for valid readings
  if (isnan(temperature) || isnan(pressure)) {
    Serial.println(F("✗ ERROR: Failed to read from sensor!"));
    delay(TEST_INTERVAL);
    return;
  }

  // Update statistics
  readingCount++;
  if (pressure < minPressure) minPressure = pressure;
  if (pressure > maxPressure) maxPressure = pressure;
  if (temperature < minTemp) minTemp = temperature;
  if (temperature > maxTemp) maxTemp = temperature;
  if (altitude < minAltitude) minAltitude = altitude;
  if (altitude > maxAltitude) maxAltitude = altitude;

  // Print readings
  Serial.print(millis());
  Serial.print("\t\t");
  Serial.print(temperature, 2);
  Serial.print("\t\t");
  Serial.print(pressure, 2);
  Serial.print("\t\t");
  Serial.println(altitude, 2);

  // Print statistics every 10 readings
  if (readingCount % 10 == 0) {
    Serial.println(F("\n--- Statistics (last 10 readings) ---"));
    Serial.print(F("Temperature: Min="));
    Serial.print(minTemp, 2);
    Serial.print(F("°C, Max="));
    Serial.print(maxTemp, 2);
    Serial.println(F("°C"));

    Serial.print(F("Pressure: Min="));
    Serial.print(minPressure, 2);
    Serial.print(F(" hPa, Max="));
    Serial.print(maxPressure, 2);
    Serial.print(F(" hPa, Range="));
    Serial.print(maxPressure - minPressure, 2);
    Serial.println(F(" hPa"));

    Serial.print(F("Altitude: Min="));
    Serial.print(minAltitude, 2);
    Serial.print(F("m, Max="));
    Serial.print(maxAltitude, 2);
    Serial.print(F("m, Range="));
    Serial.print(maxAltitude - minAltitude, 2);
    Serial.println(F("m"));

    Serial.println(F("--------------------------------------\n"));

    // Reset statistics
    minPressure = 999999;
    maxPressure = 0;
    minTemp = 999999;
    maxTemp = -999999;
    minAltitude = 999999;
    maxAltitude = -999999;
  }

  // Validation checks
  if (readingCount == 5) {
    Serial.println(F("\n========================================"));
    Serial.println(F("Running validation checks..."));
    Serial.println(F("========================================"));

    // Check 1: Temperature range
    if (temperature < -40 || temperature > 85) {
      Serial.println(F("⚠ WARNING: Temperature out of normal range!"));
      Serial.println(F("  Expected: -40°C to 85°C"));
    } else {
      Serial.println(F("✓ Temperature in valid range"));
    }

    // Check 2: Pressure range
    if (pressure < 300 || pressure > 1100) {
      Serial.println(F("⚠ WARNING: Pressure out of normal range!"));
      Serial.println(F("  Expected: 300-1100 hPa"));
    } else {
      Serial.println(F("✓ Pressure in valid range"));
    }

    // Check 3: Room temperature check
    if (temperature > 15 && temperature < 30) {
      Serial.println(F("✓ Temperature reasonable for indoor testing"));
    } else {
      Serial.println(F("⚠ Temperature unusual for room conditions"));
    }

    // Check 4: Sea level pressure check
    if (pressure > 950 && pressure < 1050) {
      Serial.println(F("✓ Pressure reasonable for sea level"));
    } else if (pressure < 950) {
      Serial.println(F("ℹ Pressure low - you may be at high altitude"));
      float estimatedAltitude = 44330 * (1.0 - pow(pressure / 1013.25, 0.1903));
      Serial.print(F("  Estimated altitude: "));
      Serial.print(estimatedAltitude, 0);
      Serial.println(F(" meters"));
    }

    Serial.println(F("========================================\n"));
  }

  // Wait for next reading
  delay(TEST_INTERVAL);
}
