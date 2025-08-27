#include <OneWire.h>
#include <DallasTemperature.h>
#include "temp.h"

#define ONE_WIRE_BUS 8  // Data pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void initTempSensor() {
  sensors.begin();
  Serial.println(F("Temp sensor setup complete"));
}

float readTemperatureC() {
  sensors.requestTemperatures();                 // Request data
  float tempC = sensors.getTempCByIndex(0);      // Get first sensor's reading

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: no temperature data"));
    return -999.0;  // Return error value
  } else {
    return tempC;
  }
}
