#ifndef CONFIG_H
#define CONFIG_H

// Temperature Sensor Configuration (NOT USED)
// #define TEMP_SENSOR_PIN 12  // Digital Temperature Sensor (DS18B20) - Not used in this project

// SD Card Configuration
#define SD_CS_PIN 9   // SPI CS uSD Card (D3)

// IMU Configuration (ICM-20948)
#define IMU_CS_PIN 6    // SPI CS IMU (sen_cs)

// Barometer Configuration (BMP280)
// #define BARO_CS_PIN 3    // SPI CS BARO (bmp_cs)

// SPI Configuration
#define SPI_MISO_PIN 12  // SPI MISO
#define SPI_MOSI_PIN 11  // SPI MOSI (Arduino Nano uses pin 11 for MOSI)
#define SPI_SCK_PIN 13   // SPI SCK


// Status Indicators
#define STATUS_LED_PIN 13
#define BUZZER_PIN 7



// State machine timing
#define STATE_TIMEOUT_MS 30000        // 30 second state timeout
#define SEQUENCER_UPDATE_RATE_MS 100  // 10Hz sequencer updates
#define FAST_TELEM_RATE_MS 500        // 2Hz during critical phases (0.5s)
#define NORMAL_TELEM_RATE_MS 500      // 2Hz during normal operations (0.5s)


// Launch detection
#define LAUNCH_ACCEL_THRESHOLD_G 2.0   // G-force to detect launch
#define APOGEE_VELOCITY_THRESHOLD -2.0 // Negative velocity for apogee
#define LANDING_VELOCITY_THRESHOLD 5.0 // Low velocity for landing detection
#define MINIMUM_FLIGHT_ALTITUDE_M 30.0 // Minimum altitude to be considered flight

// Legacy flight detection (for compatibility with existing code)
#define ALTITUDE_RISE_THRESHOLD_M 10.0   // Altitude increase to detect flight start
#define ALTITUDE_FALL_THRESHOLD_M 5.0    // Altitude decrease to detect landing
#define ALTITUDE_CHECK_INTERVAL_MS 1000  // How often to check for flight state changes

// ============================================================================
// SAMPLING RATES
// ============================================================================

// Flight phase sampling rates (in milliseconds)
#define PREFLIGHT_SAMPLE_RATE_MS 500     // 2 Hz before flight (0.5s)
#define FLIGHT_SAMPLE_RATE_MS 500        // 2 Hz during flight (0.5s)
#define POSTFLIGHT_SAMPLE_RATE_MS 500    // 2 Hz after flight (0.5s)

// ============================================================================
// DATA LOGGING
// ============================================================================

#define LOG_FILENAME "flight.txt"
#define MAX_LOG_FILE_SIZE_MB 32

// ============================================================================
// DEBUG SETTINGS
// ============================================================================

#define SERIAL_BAUD_RATE 115200
#define ENABLE_SERIAL_DEBUG true

// ============================================================================
// ERROR HANDLING
// ============================================================================

#define MAX_SENSOR_RETRIES 3
#define SENSOR_TIMEOUT_MS 5000

#endif // CONFIG_H

