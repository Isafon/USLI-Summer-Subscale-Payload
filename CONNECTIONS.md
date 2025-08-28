# Sensor Connection Guide

## Arduino Nano v4 Pinout

### SPI Pins (for SD Card)
- **MOSI**: Pin 11 (Master Out Slave In)
- **MISO**: Pin 12 (Master In Slave Out) 
- **SCK**: Pin 13 (Serial Clock)
- **CS**: Pin 10 (Chip Select) - **This is the issue!**

### Digital Pins
- **Pin 0**: GPS RX (Software Serial)
- **Pin 1**: GPS TX (Software Serial)
- **Pin 8**: Temperature Sensor Data
- **Pin 10**: SD Card CS (not pin 11!)
- **Pin 11**: SPI MOSI
- **Pin 12**: SPI MISO
- **Pin 13**: SPI SCK + Built-in LED

## Current Issues Found

### 1. SD Card Connection Problem
**Current Config**: CS pin 11 ❌
**Correct Config**: CS pin 9 ✅

**Fix**: Update SD card CS pin from 11 to 9

### 2. Temperature Sensor Connection
**Current Status**: Not responding
**Check**: 
- Data pin → Arduino pin 8
- VCC → 5V
- GND → GND
- 4.7kΩ pull-up resistor between data and VCC

### 3. GPS Connection
**Current Status**: Working ✅
**Note**: 0 satellites is normal indoors

## Corrected Pin Configuration

### SD Card Module
```
SD Card → Arduino Nano
VCC    → 5V
GND    → GND
CS     → Pin 9 (uSD_CS)
MOSI   → Pin 11 (SPI_MOSI)
MISO   → Pin 12 (SPI_MISO)
SCK    → Pin 13 (SPI_CLK)
```

### Temperature Sensor (DS18B20)
```
DS18B20 → Arduino Nano
VCC     → 5V
GND     → GND
Data    → Pin 8
4.7kΩ   → Between Data and VCC
```

### GPS Module (NEO-6M)
```
NEO-6M → Arduino Nano
VCC    → 5V
GND    → GND
TX     → Pin 0 (RX)
RX     → Pin 1 (TX)
```

## Steps to Fix

1. **Update SD Card CS pin** in config.h
2. **Check temperature sensor wiring**
3. **Verify all connections**
4. **Test again**

## Testing Commands

```bash
# Deploy and test
./arduino.sh deploy

# Look for these success messages:
# ✅ SD card initialized successfully
# ✅ Temperature sensor working: XX.X°C
# GPS: Waiting for satellite fix...
```
