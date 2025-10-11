# Testing Guide - USLI Summer Subscale Payload

This guide provides step-by-step instructions for testing all components on a breadboard.

## Overview

You have 7 new test sketches to validate your system:

1. **I2C Scanner** - Detect I2C devices on the bus
2. **Barometer Test** - Test BMP280 sensor
3. **IMU Test** - Test ICM-20948 sensor
4. **Hardware Control Test** - Test pyrotechnic channels (with LEDs)
5. **Flight Detection Test** - Test flight state algorithms
6. **System Integration Test** - Test all sensors together
7. **Sequencer Test** - Test complete mission sequence

## Breadboard Testing Setup

### Important Notes:
- **ICM-20948 requires 3.3V ONLY!** Do not connect to 5V
- BMP280 can use 3.3V or 5V (check your module)
- All I2C devices share the same SDA/SCL lines (A4/A5 on Arduino Nano)
- Use jumper wires - connections may be flaky, push firmly

### Pin Mapping Reference

| Component | Arduino Pin | Connection |
|-----------|-------------|------------|
| GPS RX | Pin 0 (Arduino TX) | GPS module RX |
| GPS TX | Pin 1 (Arduino RX) | GPS module TX |
| SD CS | Pin 11 | SD card module CS |
| Temp Data | Pin 12 | DS18B20 data line |
| I2C SDA | A4 | IMU & Barometer SDA |
| I2C SCL | A5 | IMU & Barometer SCL |
| Pyro Channels | Pins 2,4,5,7,A0 | LEDs for testing |
| Status LED | Pin 13 | Built-in LED |

---

## Testing Sequence

### Test 1: I2C Scanner (Start Here!)

**Purpose:** Verify I2C wiring and detect all devices

**Hardware:** Connect IMU and Barometer to breadboard

**Steps:**
```bash
cd examples/i2c_scanner
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Expected Output:**
```
I2C device found at address 0x69 (ICM-20948 IMU)
I2C device found at address 0x76 (BMP280 Barometer)
Found 2 device(s)
```

**Troubleshooting:**
- No devices found? Check power connections (VCC and GND)
- Check SDA -> A4 and SCL -> A5 connections
- Verify ICM-20948 is connected to 3.3V (NOT 5V!)
- Most modules have built-in pullup resistors

---

### Test 2: Temperature Sensor

**Purpose:** Verify DS18B20 temperature sensor

**Hardware:** Connect DS18B20 to Pin 12 with 4.7kΩ pullup resistor

**Steps:**
```bash
cd examples/temp
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Expected Output:**
```
Found 1 devices.
Device 0 Address: 28XXXXXXXX
Temp C: 22.50 Temp F: 72.50
```

**What to Verify:**
- Temperature is reasonable (room temp ~20-25°C)
- Reading updates every 1.5 seconds
- No "Device disconnected" errors

---

### Test 3: Barometer (BMP280)

**Purpose:** Test pressure, temperature, and altitude readings

**Hardware:** BMP280 connected via I2C (found by scanner)

**Steps:**
```bash
cd examples/baro
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Expected Output:**
```
✓ BMP280 detected at address 0x76
✓ BMP280 initialized successfully!

Time(ms)    Temp(°C)    Pressure(hPa)    Altitude(m)
1000        22.35       1013.25          0.00
2000        22.36       1013.24          0.05
```

**What to Verify:**
- Pressure around 1013 hPa (at sea level) or lower at altitude
- Temperature reasonable for room conditions
- Altitude near 0m (or your actual elevation)
- Statistics show minimal drift

**Try This:**
- Blow on sensor -> temperature should increase
- Cover sensor and blow -> pressure should increase slightly

---

### Test 4: IMU (ICM-20948)

**Purpose:** Test accelerometer, gyroscope, and magnetometer

**Hardware:** ICM-20948 connected via I2C at **3.3V**

**Steps:**
```bash
cd examples/IMU
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Expected Output:**
```
✓ ICM-20948 detected at address 0x69
✓ ICM-20948 initialized successfully!

Reading #1 @ 1000ms
  Accel: X=0.05 Y=0.02 Z=9.81 |Mag|=9.81
  Gyro:  X=0.12 Y=-0.05 Z=0.03
  Mag:   X=25.5 Y=12.3 Z=-32.1
  Temp:  23.5°C

✓ Accelerometer gravity check PASSED
✓ Gyroscope drift check PASSED
```

**What to Verify:**
- Acceleration magnitude ~9.81 m/s² (1G gravity)
- Gyro values near zero when stationary
- No high drift warnings
- Temperature reasonable

**Try This:**
- Rotate board -> gyro values should change
- Flip board -> Z-axis acceleration should change sign
- Move quickly -> acceleration magnitude increases
- Watch for motion detection messages

---

### Test 5: GPS

**Purpose:** Verify GPS can get satellite fix and position

**Hardware:** GPS module connected to pins 0 and 1

**Steps:**
```bash
cd examples/gps
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Expected Output:**
```
Satellites: 8
Location: 40.712800, -74.006000
Altitude: 10.5 m
```

**Important:**
- GPS requires **clear view of sky**
- May take 1-5 minutes for first fix (cold start)
- Indoors will likely NOT work
- Best tested outdoors or near window

---

### Test 6: SD Card

**Purpose:** Verify SD card can be read and written

**Hardware:** SD card module connected to Pin 11 (CS)

**Steps:**
```bash
cd examples/uSD
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**What to Verify:**
- Card initializes successfully
- Can create and write files
- Check SD card on computer afterward

**Troubleshooting:**
- Use FAT32 formatted card
- Try different SD card if one doesn't work
- Check CS pin connection (Pin 11)

---

### Test 7: Hardware Control Test

**Purpose:** Test pyrotechnic output channels (using LEDs)

**Hardware:**
- Connect LEDs with 220Ω resistors to pins 2, 4, 5, 7, A0
- Optional: Button on A1 for emergency abort

**Steps:**
```bash
cd examples/hardware_control
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Serial Commands:**
- `1` = Test booster ignition (Pin 2)
- `2` = Test nose fairing (Pin 4)
- `3` = Test stage separation (Pin 5)
- `4` = Test payload deploy (Pin 7)
- `5` = Test parachute deploy (Pin A0)
- `a` = Test all channels sequentially
- `s` = Safe all channels

**What to Verify:**
- LEDs light up for ~1 second when channel is fired
- All channels can be tested individually
- Emergency abort button works (if connected)
- Status LED blinks continuously

---

### Test 8: Flight Detection Test

**Purpose:** Test flight state detection algorithms

**Hardware:** None required (pure simulation)

**Steps:**
```bash
cd examples/flight_detection
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Serial Commands:**
- `1` = Normal flight simulation (300m apogee)
- `2` = High-altitude flight (1000m)
- `3` = Abort scenario
- `4` = Test launch threshold
- `5` = Test apogee detection

**What to Verify:**
- Launch detected at 2G acceleration
- Apogee detected when velocity goes negative
- Landing detected when altitude near zero
- All state transitions happen correctly

---

### Test 9: Full System Integration

**Purpose:** Test all sensors working together

**Hardware:** ALL sensors connected on breadboard

**Steps:**
```bash
cd examples/system_integration
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Expected Output:**
```
========================================
Initialization Summary
========================================
GPS:         ✓ OK
Temperature: ✓ OK
Barometer:   ✓ OK
IMU:         ✓ OK
SD Card:     ✓ OK

Total Sensors Working: 5 / 5
========================================

Reading #1 @ 1000ms
Temperature: 22.35 °C
Pressure: 1013.25 hPa, Altitude: 0.00 m
GPS: 40.712800, -74.006000, Alt: 10.5 m, Sats: 8
IMU Accel: 0.05, 0.02, 9.81 m/s²
IMU Gyro: 0.12, -0.05, 0.03 °/s
✓ Data logged to SD card
```

**What to Verify:**
- All 5 sensors initialize successfully
- All sensors provide valid data simultaneously
- Data is logged to SD card (check sysTest.txt)
- Loop time statistics are reasonable
- No I2C conflicts between barometer and IMU

**This is the most important test!** It validates your entire hardware setup.

---

### Test 10: Sequencer Test

**Purpose:** Test complete mission sequence simulation

**Hardware:** Optional LEDs on pyro pins to visualize events

**Steps:**
```bash
cd examples/sequencer_test
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

**Serial Commands:**
- `1` = Auto-run full mission (all 17 states)
- `2` = Manual step mode (press 's' to advance)
- `3` = Test emergency abort
- `r` = Reset sequencer

**What to Verify:**
- All 17 states execute in correct order
- Phase transitions happen correctly
- Launch detected at 2G threshold
- Apogee detected properly
- All deployment events fire
- Status LED blink rate changes with phase
- Mission completes successfully

**This validates your flight software logic!**

---

## Common Issues and Solutions

### I2C Issues

**Problem:** No devices detected by I2C scanner

**Solutions:**
- Verify SDA -> A4 and SCL -> A5
- Check power connections (VCC and GND)
- ICM-20948 must use 3.3V, not 5V
- Try different jumper wires (can be flaky)
- Most modules have built-in pullup resistors

---

### Temperature Sensor Issues

**Problem:** "Unable to find address for Device 0"

**Solutions:**
- Check 4.7kΩ pullup resistor between Data and VCC
- Verify wiring: Data -> Pin 12, VCC -> 5V, GND -> GND
- Try different DS18B20 sensor

---

### SD Card Issues

**Problem:** "Card not detected"

**Solutions:**
- Format card as FAT32
- Try different SD card
- Check CS pin connection (Pin 11)
- Some SD modules need 5V, some need 3.3V

---

### GPS Issues

**Problem:** "No fix"

**Solutions:**
- GPS needs clear view of sky (won't work indoors usually)
- Wait 1-5 minutes for cold start
- Test outdoors or near window
- Check antenna connection
- Verify baud rate (usually 9600)

---

## Test Results Checklist

Use this checklist to track your progress:

- [ ] I2C Scanner detects both IMU and Barometer
- [ ] Temperature sensor reads room temperature
- [ ] Barometer reads reasonable pressure/altitude
- [ ] IMU reads ~9.81 m/s² gravity when stationary
- [ ] GPS gets satellite fix (if outdoors)
- [ ] SD card can write and read files
- [ ] Hardware control LEDs light up correctly
- [ ] Flight detection simulation runs successfully
- [ ] System integration test - all 5 sensors work together
- [ ] Sequencer test completes full mission simulation

---

## Next Steps

Once all tests pass:

1. **Validate data logging** - Check SD card files on computer
2. **Test with movement** - Shake/rotate IMU, watch values change
3. **Test outdoors** - Get GPS fix and log real coordinates
4. **Calibrate sensors** - Record baseline values at rest
5. **Integration with main app** - Use components in main.cpp
6. **Flight readiness** - Replace test LEDs with actual pyrotechnics (carefully!)

---

## Safety Reminders

- **NEVER connect actual pyrotechnics during testing**
- Always use LEDs or buzzers to simulate deployments
- Keep emergency abort accessible during tests
- Test in safe environment away from flammable materials
- Double-check wiring before powering on
- ICM-20948 is **3.3V ONLY** - 5V will damage it!

---

## Quick Reference: Upload and Monitor

```bash
# Template for testing any example
cd examples/<test_name>
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200

# Or use your arduino.sh script
./arduino.sh build
./arduino.sh upload
./arduino.sh monitor
```

---

## Support

If you encounter issues:
1. Check wiring against pin diagrams
2. Run I2C scanner to verify connections
3. Check serial output for error messages
4. Verify power supply is adequate (500mA+ recommended)
5. Try components individually before integration

Good luck with your testing!
