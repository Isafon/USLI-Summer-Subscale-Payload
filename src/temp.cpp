#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 8  // Data pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();
  Serial.println(F("Temp sensor setup complete "));
}

void loop() {
  sensors.requestTemperatures();                 // Request data
  float tempC = sensors.getTempCByIndex(0);      // Get first sensor’s reading

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: no temperature data"));
  } else {
    Serial.print(F("Temp C: "));
    Serial.println(tempC);
  }

  delay(1000);  // 1 Hz sample
}
