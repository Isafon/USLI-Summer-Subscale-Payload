# USLI-Summer-Subscale-Payload
USLI 25-26 Subscale rocket payload software for Summer 2025. Collects, stores, and transmits flight telemetry using STM32 and onboard sensors.

## 🚀 Overview
This repository contains the firmware and support tools for the subscale rocket payload developed during the Summer pre-senior design program. The payload collects flight telemetry data using onboard sensors and stores or transmits it for later analysis.

## 📦 Payload Components
- **Microcontroller**: STM32F103 "Blue Pill"
- **Sensors**:
  - [BN008x] IMU (Acceleration + Gyroscope)
  - [BMP280/BMP388] Barometric Altimeter + Temperature
  - [DS18B20] (Temperature)
  - [NEO-6M] GPS Module (TBD)
- **Storage**: [MicroSD card]
- **Transmission**: LoRa / HC-12 RF module (TBD)

## 🧠 Software Architecture
- **Language**: C / C++
- **IDE**: STM32CubeIDE or PlatformIO
- **Core Features**:
  - Sensor data acquisition
  - Structured telemetry packet creation
  - CSV logging to SD card
  - Real-time transmission via RF module
  - Ground station reception and decoding

## 📁 Folder Structure
- [TBD]
