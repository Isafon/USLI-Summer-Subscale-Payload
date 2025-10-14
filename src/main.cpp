#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP280.h>
#include "rtc_pcf8523.h"   // your working wrapper (initRTC, readRTC, setRTC)

// ---------- Pins (UNO) ----------
#define BMP_CS  5           // BMP280 chip select
#define SD_CS   9           // microSD chip select (use your actual wiring)

// ---------- Constants ----------
#define LOG_INTERVAL_MS     1000
#define SEA_LEVEL_PRESSURE  1013.25f  // hPa

// ---------- Globals ----------
Adafruit_BMP280 bmp;        // use HW SPI: begin(BMP_CS)
File logFile;

// ---------- SPI hygiene ----------
static inline void deselectAllSpi() {
  digitalWrite(BMP_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
}

static inline bool readBMP(float &tempC, float &press_hPa) {
  // make sure SD is not selected while we talk to BMP
  digitalWrite(SD_CS, HIGH);
  digitalWrite(BMP_CS, LOW);
  delayMicroseconds(2);

  tempC    = bmp.readTemperature();
  press_hPa = bmp.readPressure() / 100.0f;

  digitalWrite(BMP_CS, HIGH);
  return !(isnan(tempC) || isnan(press_hPa));
}

static inline float pressureToAltitude(float p_hPa) {
  return 44330.0f * (1.0f - powf(p_hPa / SEA_LEVEL_PRESSURE, 0.1903f));
}

// ---------- RTC timestamp ----------
bool getTimestamp(char* buf, size_t n) {
  DateTime dt;
  if (readRTC(dt) && dt.dataValid) {
    snprintf(buf, n, "%04d-%02d-%02d %02d:%02d:%02d",
             dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    return true;
  }
  return false;
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // I2C first (stable, slow)
  Wire.begin();
  Wire.setClock(100000);       // 100 kHz
  delay(50);

  if (!initRTC()) {
    Serial.println(F("RTC init failed (wiring/3V3/SDA=A4,SCL=A5?)."));
    while (1) delay(1000);
  }

  // If the RTC has default/invalid time, set it once (or use your RTC tool)
  DateTime now;
  if (readRTC(now) && now.year < 2024) {
    setRTC(2025, 10, 12, 17, 0, 0);
    delay(50);
  }

  // SPI + CS pins
  pinMode(10, OUTPUT);         // SS must be OUTPUT or SPI goes slave
  pinMode(BMP_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  deselectAllSpi();
  delay(5);

  // BMP280 (HW SPI)
  if (!bmp.begin(BMP_CS)) {
    Serial.println(F("BMP init failed (check CS=5, 3V3, GND, SPI lines)."));
    while (1) delay(1000);
  }
  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,     // Temp oversample
    Adafruit_BMP280::SAMPLING_X16,    // Pressure oversample
    Adafruit_BMP280::FILTER_X16,      // Strong IIR
    Adafruit_BMP280::STANDBY_MS_500
  );

  // SD LAST
  Serial.print(F("SD init (CS="));
  Serial.print(SD_CS);
  Serial.print(F(")... "));
  deselectAllSpi();
  digitalWrite(SD_CS, LOW);
  bool sdok = SD.begin(SD_CS);
  digitalWrite(SD_CS, HIGH);
  Serial.println(sdok ? F("OK") : F("FAILED"));
  if (!sdok) while (1) delay(1000);

  // Open log
  deselectAllSpi();
  digitalWrite(SD_CS, LOW);
  logFile = SD.open("data_log.csv", FILE_WRITE);
  digitalWrite(SD_CS, HIGH);
  if (!logFile) {
    Serial.println(F("Log open failed"));
    while (1) delay(1000);
  }
  if (logFile.size() == 0) {
    digitalWrite(SD_CS, LOW);
    logFile.println(F("Timestamp,Temp_C,Pressure_hPa,Altitude_m"));
    logFile.flush();
    digitalWrite(SD_CS, HIGH);
  }

  Serial.println(F("Ready."));
}

// ---------- Loop ----------
void loop() {
  static unsigned long tLast = 0;
  if (millis() - tLast < LOG_INTERVAL_MS) return;
  tLast = millis();

  // BMP read (with SD deselected)
  float T = NAN, P = NAN;
  if (!readBMP(T, P) || T < -40 || T > 85 || P < 300 || P > 1100) {
    delay(10);                // one quick retry
    readBMP(T, P);
    if (isnan(T) || isnan(P) || T < -40 || T > 85 || P < 300 || P > 1100) {
      T = 0.0f; P = 0.0f;
    }
  }
  float alt = (P > 0) ? pressureToAltitude(P) : 0.0f;

  // RTC
  char ts[32];
  if (!getTimestamp(ts, sizeof(ts))) strcpy(ts, "RTC_ERROR");

  // Serial
  Serial.print("["); Serial.print(ts); Serial.print("] ");
  Serial.print("T="); Serial.print(T, 2); Serial.print(" C, ");
  Serial.print("P="); Serial.print(P, 2); Serial.print(" hPa, ");
  Serial.print("Alt="); Serial.print(alt, 2); Serial.println(" m");

  // Log (select SD only for the write)
  if (logFile) {
    deselectAllSpi();
    digitalWrite(SD_CS, LOW);
    logFile.print(ts); logFile.print(',');
    logFile.print(T, 2); logFile.print(',');
    logFile.print(P, 2); logFile.print(',');
    logFile.println(alt, 2);
    logFile.flush();
    digitalWrite(SD_CS, HIGH);
  }
}