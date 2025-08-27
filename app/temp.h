#ifndef TEMP_H
#define TEMP_H

#include <Arduino.h>

// Initialize the temperature sensor
void initTempSensor();

// Read temperature in Celsius from the first device
float readTemperatureC();

#endif // TEMP_H
