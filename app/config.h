#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// PIN CONFIGURATIONS
// ============================================================================

// GPS Configuration
#define GPS_RX_PIN 0  // GPS RX
#define GPS_TX_PIN 1  // GPS TX
#define GPS_BAUD_RATE 4800

// Temperature Sensor Configuration
#define TEMP_SENSOR_PIN 8  // Analog Temperature Sensor

// SD Card Configuration
#define SD_CS_PIN 11  // SPI CS uSD Card

// IMU Configuration (placeholder - update when IMU is implemented)
#define IMU_CS_PIN 6    // SPI CS IMU

// Barometer Configuration (placeholder - update when baro is implemented)
#define BARO_CS_PIN 3    // SPI CS BARO

// SPI Configuration
#define SPI_MISO_PIN 12  // SPI MISO
#define SPI_MOSI_PIN 11  // SPI MOSI
#define SPI_SCK_PIN 13   // SPI SCK
//#define SPI_SS_PIN 10    // SPI CS

// ============================================================================
// SAMPLING RATES
// ============================================================================

// Flight phase sampling rates (in milliseconds)
#define PREFLIGHT_SAMPLE_RATE_MS 1000    // 1 Hz before flight
#define FLIGHT_SAMPLE_RATE_MS 100        // 10 Hz during flight
#define POSTFLIGHT_SAMPLE_RATE_MS 5000   // 0.2 Hz after flight

// ============================================================================
// FLIGHT DETECTION PARAMETERS
// ============================================================================

#define ALTITUDE_RISE_THRESHOLD_M 10.0   // Altitude increase to detect flight start
#define ALTITUDE_FALL_THRESHOLD_M 5.0    // Altitude decrease to detect landing
#define ALTITUDE_CHECK_INTERVAL_MS 1000  // How often to check for flight state changes

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
