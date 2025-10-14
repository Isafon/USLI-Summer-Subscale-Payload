#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "rtc_pcf8523.h"
#include <SD.h>

#define BMP_CS 3
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK 13
#define SEA_LEVEL_PRESSURE 1013.25

// Create BMP280 object exactly like working example
Adafruit_BMP280 bmp(BMP_CS, SPI_MOSI, SPI_MISO, SPI_SCK);

bool on,r,b,s;
unsigned long n,t;

// Plausibility check from working baro example
bool validReading(float temp, float press) {
  if (isnan(temp) || isnan(press)) return false;
  if (temp < -40 || temp > 85) return false;
  if (press < 300 || press > 1100) return false;
  return true;
}

// Compute altitude from pressure (from working example)
float calculateAltitude(float pressure_hPa, float seaLevel_hPa) {
  return 44330.0 * (1.0 - pow(pressure_hPa / seaLevel_hPa, 0.1903));
}

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(10);
  pinMode(13,OUTPUT);
  
  Serial.println("=== Logger ===");
  
  Wire.begin();
  r=initRTC();
  Serial.print("RTC:");
  Serial.println(r?"OK":"FAIL");
  
  // Set RTC time if needed (from working RTC example)
  DateTime currentTime;
  if (readRTC(currentTime)) {
    Serial.print("RTC reads: ");
    Serial.print(currentTime.year);
    Serial.print("-");
    Serial.print(currentTime.month);
    Serial.print("-");
    Serial.print(currentTime.day);
    Serial.print(" ");
    Serial.print(currentTime.hour);
    Serial.print(":");
    Serial.print(currentTime.minute);
    Serial.print(":");
    Serial.println(currentTime.second);
    
    bool isDefaultTime = (currentTime.hour == 12 && currentTime.minute < 5);
    if (currentTime.year < 2024 || isDefaultTime) {
      Serial.println("Setting RTC time...");
      // Set to current time (modify as needed)
      Wire.beginTransmission(0x68);
      Wire.write(0x03); // seconds register
      Wire.write(0x00); // seconds
      Wire.write(0x00); // minutes  
      Wire.write(0x12); // hours (18:00)
      Wire.write(0x12); // day
      Wire.write(0x00); // weekday
      Wire.write(0x10); // month
      Wire.write(0x25); // year (2025)
      Wire.endTransmission();
      delay(100);
      
      // Read back to verify
      if (readRTC(currentTime)) {
        Serial.print("New RTC time: ");
        Serial.print(currentTime.year);
        Serial.print("-");
        Serial.print(currentTime.month);
        Serial.print("-");
        Serial.print(currentTime.day);
        Serial.print(" ");
        Serial.print(currentTime.hour);
        Serial.print(":");
        Serial.print(currentTime.minute);
        Serial.print(":");
        Serial.println(currentTime.second);
      }
    }
  }
  
  SPI.begin();
  
  // Set up SPI pins explicitly
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);
  pinMode(SPI_SCK, OUTPUT);
  pinMode(BMP_CS, OUTPUT);
  digitalWrite(BMP_CS, HIGH);
  
  Serial.print("Baro:");
  // Initialize BMP280 exactly like working example
  Serial.println("Initializing BMP280 (SPI mode)...");
  Serial.print("CS Pin: "); Serial.println(BMP_CS);
  Serial.print("MOSI: "); Serial.println(SPI_MOSI);
  Serial.print("MISO: "); Serial.println(SPI_MISO);
  Serial.print("SCK: "); Serial.println(SPI_SCK);
  
  if (!bmp.begin()) {
    Serial.println("FAIL - Check wiring!");
    b = false;
  } else {
    Serial.println("OK");
    b = true;
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,Adafruit_BMP280::SAMPLING_X2,Adafruit_BMP280::SAMPLING_X16,Adafruit_BMP280::FILTER_X16,Adafruit_BMP280::STANDBY_MS_500);
    Serial.println("BMP280 configured successfully!");
  }
  
  Serial.print("SD:");
  s=SD.begin(9);
  Serial.println(s?"OK":"FAIL");
  
  if(!s) {
    Serial.println("ERROR: No SD card!");
    while(1) {
      digitalWrite(13,HIGH);
      delay(200);
      digitalWrite(13,LOW);
      delay(200);
    }
  }
  
  Serial.println("L=start S=stop");
}

void loop() {
  if(Serial.available()) {
    char c=Serial.read();
    while(Serial.available()) Serial.read();
    
    if(c=='L'||c=='l') {
      on=1; n=0; t=millis();
      Serial.println("ON");
    }
    else if(c=='S'||c=='s') {
      on=0;
      Serial.print("Recs:");
      Serial.println(n);
    }
  }
  
  if(on&&(millis()-t>=1000)) {
    t=millis();
    
    char buf[60];
    int p=0;
    
    // Get timestamp from RTC
    DateTime dt;
    if(r&&readRTC(dt)&&dt.dataValid) {
      // Format: YYYY-MM-DD HH:MM:SS
      p+=snprintf(buf+p,60-p,"%04d-%02d-%02d %02d:%02d:%02d,",dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second);
    } else {
      p+=snprintf(buf+p,60-p,"%lu,",millis());
    }
    
    // Get baro data exactly like working example
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F; // Pa → hPa
    
    if (!validReading(temperature, pressure)) {
      delay(10);
      temperature = bmp.readTemperature();
      pressure = bmp.readPressure() / 100.0F;
      if (!validReading(temperature, pressure)) {
        p+=snprintf(buf+p,60-p,"NaN,NaN,NaN");
      } else {
        float altitude = calculateAltitude(pressure, SEA_LEVEL_PRESSURE);
        p+=snprintf(buf+p,60-p,"%.1f,%.1f,%.1f",temperature,pressure,altitude);
      }
    } else {
      float altitude = calculateAltitude(pressure, SEA_LEVEL_PRESSURE);
      p+=snprintf(buf+p,60-p,"%.1f,%.1f,%.1f",temperature,pressure,altitude);
    }
    
    File f=SD.open("d.csv",FILE_WRITE);
    if(f) {
      f.println(buf);
      f.close();
      n++;
      Serial.println(buf);
      digitalWrite(13,HIGH);
      delay(10);
      digitalWrite(13,LOW);
    }
  }
}