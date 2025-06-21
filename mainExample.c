#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Define the telemetry packet */
typedef struct {
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    float pressure;
    float temperature;
    float altitude;
    float gps_lat, gps_lon, gps_alt;
    uint32_t timestamp_ms;
} TelemetryPacket;

TelemetryPacket currentPacket;

/* Function prototypes */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_USART1_UART_Init(void);
void initSensors(void);
void collectTelemetry(TelemetryPacket *packet);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    initSensors();

    while (1) {
        collectTelemetry(&currentPacket);
        // Store to SD or send via RF
        HAL_Delay(100); // Sample every 100ms
    }
}

void initSensors(void) {
    // Placeholder: Init IMU, Baro, Temp, GPS
}

void collectTelemetry(TelemetryPacket *packet) {
    packet->timestamp_ms = HAL_GetTick();

    // Example: populate with dummy values or real sensor reads
    packet->accel_x = 0.0;
    packet->gyro_x = 0.0;
    packet->temperature = 25.0;
    // Add baro, GPS, etc. here
}
