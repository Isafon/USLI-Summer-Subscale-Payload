/*
 * Ultra-Compact Data Logger
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "rtc_pcf8523.h"
#include "baro_bmp280.h"
#include "imu_icm20948.h"
#include "uSD.h"

#define L 1000
#define P 13

bool log_on = false;
bool rtc, baro, imu, sd;
unsigned long n = 0, t = 0;

void setup() {
  Serial.begin(115200);
  pinMode(P, OUTPUT);
  
  Wire.begin();
  rtc = initRTC();
  
  SPI.begin();
  baro = initBaro();
  imu = initIMU();
  sd = initSD();
  
  if (!sd) while(1) { digitalWrite(P, !digitalRead(P)); delay(200); }
  
  Serial.println(F("L=Log S=Stop"));
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    while(Serial.available()) Serial.read();
    
    if (c == 'L' || c == 'l') {
      log_on = true;
      n = 0;
      t = millis();
      Serial.println(F("ON"));
    }
    else if (c == 'S' || c == 's') {
      log_on = false;
      digitalWrite(P, LOW);
      Serial.println(n);
    }
  }
  
  if (log_on && (millis() - t >= L)) {
    t = millis();
    
    char buf[100];
    int pos = 0;
    
    // Timestamp
    DateTime dt;
    if (rtc && readRTC(dt)) {
      pos += snprintf(buf+pos, 100-pos, "%04d-%02d-%02d %02d:%02d:%02d,",
               dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    } else {
      pos += snprintf(buf+pos, 100-pos, "%lu,", millis());
    }
    
    // Baro
    BaroData b;
    if (baro && readBaro(b)) {
      pos += snprintf(buf+pos, 100-pos, "%.1f,%.1f,%.1f,",
               b.temperature, b.pressure, b.altitude);
    } else {
      pos += snprintf(buf+pos, 100-pos, ",,,");
    }
    
    // IMU
    IMUData i;
    if (imu && readIMU(i)) {
      pos += snprintf(buf+pos, 100-pos, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
               i.accel_x, i.accel_y, i.accel_z, i.gyro_x, i.gyro_y, i.gyro_z);
    }
    
    // Write to SD
    File f = SD.open("d.csv", FILE_WRITE);
    if (f) {
      f.println(buf);
      f.close();
      n++;
      digitalWrite(P, HIGH);
      if (n % 10 == 0) Serial.println(n);
      delay(10);
      digitalWrite(P, LOW);
    }
  }
  
  if (!log_on) {
    static unsigned long lb = 0;
    if (millis() - lb > 1000) {
      digitalWrite(P, !digitalRead(P));
      lb = millis();
    }
  }
}
