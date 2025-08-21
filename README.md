# USLI-Summer-Subscale-Payload
USLI 25-26 Subscale rocket payload firmware for Summer 2025. Payload collects and stores flight telemetry data using Arduino Nano and onboard sensors.

## 🚀 Overview
This repository contains the firmware and support tools for the subscale rocket payload developed during the Summer pre-senior design program. The payload collects flight telemetry data using onboard sensors and stores it for later analysis.

## 📦 Payload Components
- **Microcontroller**: Arduino Nano
- **Sensors**:
  - [BN008x] IMU (Acceleration + Gyroscope)
  - [BMP280/BMP388] Barometric Altimeter + Temperature
  - [DS18B20] (Temperature) - Extra Info: https://github.com/milesburton/Arduino-Temperature-Control-Library
  - [NEO-6M] GPS Module
- **Storage**: [MicroSD card] - Tutorial: https://learn.adafruit.com/adafruit-micro-sd-breakout-board-card-tutorial/arduino-library
- **Transmission**: LoRa / HC-12 RF module (TBD)

## 🧠 Software Architecture
- **Language**: C / C++
- **IDE**: Arduino IDE for simplicity
- **Core Features**:
  - Sensor data acquisition
  - Structured telemetry packet creation
  - CSV logging to SD card
  - Real-time transmission via RF module
  - Ground station reception and decoding

## 📁 Folder Structure
- [TBD]
