/*
 * Test: RTC + Baro + SD (No IMU due to memory)
 */

 #include <Arduino.h>
 #include <Wire.h>
 #include <SPI.h>
 #include "rtc_pcf8523.h"
 #include "baro_bmp280.h"
 #include "uSD.h"
 
 bool on;
 bool r,b,s;
 unsigned long n,t;
 
 void setup() {
   Serial.begin(115200);
   while(!Serial) delay(10);
   pinMode(13, OUTPUT);
   
   Serial.println(F("\n=== Test ==="));
   
   Wire.begin();
   delay(100);
   r = initRTC();
   Serial.print(F("RTC:"));
   Serial.println(r?F("OK"):F("FAIL"));
   
   SPI.begin();
   delay(100);
   b = initBaro();
   Serial.print(F("Baro:"));
   Serial.println(b?F("OK"):F("FAIL"));
   
   s = initSD();
   Serial.print(F("SD:"));
   Serial.println(s?F("OK"):F("FAIL"));
   
   if(!s) while(1) delay(999);
   
   Serial.println(F("\nL=start S=stop"));
 }
 
 void loop() {
   if (Serial.available()) {
     char c = Serial.read();
     while(Serial.available()) Serial.read();
     
     if (c=='L'||c=='l') {
       on=1; n=0; t=millis();
       Serial.println(F("ON"));
     }
     else if (c=='S'||c=='s') {
       on=0;
       Serial.print(F("Recs:"));
       Serial.println(n);
     }
   }
   
   if (on && (millis()-t>=1000)) {
     t=millis();
     
     char buf[80];
     int p=0;
     
     DateTime dt;
     if (r&&readRTC(dt)) {
       p+=snprintf(buf+p,80-p,"%04d-%02d-%02d %02d:%02d:%02d,",
                dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second);
     } else {
       p+=snprintf(buf+p,80-p,"%lu,",millis());
     }
     
     BaroData bd;
     if (b&&readBaro(bd)) {
       p+=snprintf(buf+p,80-p,"%.2f,%.1f,%.1f",
                bd.temperature,bd.pressure,bd.altitude);
     } else {
       p+=snprintf(buf+p,80-p,"NaN,NaN,NaN");
     }
     
     File f=SD.open("d.csv",FILE_WRITE);
     if (f) {
       f.println(buf);
       f.close();
       n++;
       Serial.println(buf);
     }
   }
 }
 