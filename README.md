# USLI Summer Subscale Payload - Rocket Telemetry System

This project implements a comprehensive telemetry system for rocket flight data collection. The system integrates multiple sensors to collect environmental and positional data during rocket flights, storing all data to an SD card for post-flight analysis.

## Project Structure

```
USLI-Summer-Subscale-Payload/
├── app/
│   ├── main.cpp          # Main application file
│   └── config.h          # Configuration and pin definitions
├── include/
│   ├── baro.h            # Barometer sensor interface
│   ├── gps.h             # GPS sensor interface
│   ├── imu.h             # IMU sensor interface
│   ├── temp.h            # Temperature sensor interface
│   └── uSD.h             # Micro SD card interface
├── src/
│   ├── baro.cpp          # Barometer implementation
│   ├── gps.cpp           # GPS implementation
│   ├── imu.cpp           # IMU implementation
│   ├── temp.cpp          # Temperature sensor implementation
│   └── uSD.cpp           # SD card implementation
├── examples/             # Individual sensor examples
└── docs/                 # Documentation and setup images
```

## Hardware Requirements

### Sensors
- **GPS Module**: NEO-6M or similar (connected to pins 0,1)
- **Temperature Sensor**: DS18B20 (connected to pin 12)
- **IMU**: MPU6050 or similar (I2C - pins 20,21)
- **Barometer**: BMP280 or similar (I2C - pins 20,21)
- **SD Card Module**: Connected to pin 11 (CS)

### Microcontroller
- Arduino Nano or similar Arduino-compatible board

## Pin Configuration

| Component | Pin | Description |
|-----------|-----|-------------|
| GPS RX    | 0   | GPS data receive |
| GPS TX    | 1   | GPS data transmit |
| SD CS     | 11  | SD card chip select |
| Temp Data | 12  | DS18B20 data line |
| I2C SDA   | 20  | I2C data line (IMU/Baro) |
| I2C SCL   | 21  | I2C clock line (IMU/Baro) |

## Features

### Flight State Detection
The system automatically detects different phases of flight:
- **Preflight**: 1 Hz sampling, waiting for launch
- **Flight**: 10 Hz sampling, during active flight
- **Postflight**: 0.2 Hz sampling, after landing

### Data Collection
The system collects the following telemetry data:
- Timestamp (milliseconds since power-on)
- Temperature (°C)
- Pressure (Pa)
- Altitude (m)
- GPS coordinates (latitude, longitude)
- GPS altitude (m)
- Number of GPS satellites
- Accelerometer data (X, Y, Z)
- Gyroscope data (X, Y, Z)
- Flight state

### Data Logging
All data is logged to `flight.txt` on the SD card in CSV format with headers for easy analysis.

## Setup Instructions

### 1. Hardware Assembly
1. Connect all sensors according to the pin configuration above
2. Insert a formatted SD card (FAT32) into the SD card module
3. Ensure proper power supply (5V recommended)

### 2. Software Setup
1. Install required Arduino libraries:
   - TinyGPS++
   - OneWire
   - DallasTemperature
   - SD (included with Arduino)

2. Configure the system in `app/config.h`:
   - Adjust pin assignments if needed
   - Modify sampling rates
   - Set flight detection thresholds

### 3. Upload and Test
1. Upload the code to your Arduino
2. Open Serial Monitor at 115200 baud
3. Verify all sensors initialize properly
4. Check that data is being logged to SD card

## Usage

### Preflight
- Power on the system
- Wait for GPS satellite fix
- Verify all sensors are reading correctly
- System will log at 1 Hz until flight detection

### During Flight
- System automatically detects launch based on altitude change
- Switches to 10 Hz sampling during flight
- All data logged to SD card

### Postflight
- System detects landing and switches to low-power mode
- Continues logging at 0.2 Hz for recovery tracking

## Data Analysis

The logged data in `flight.txt` can be imported into:
- Excel/Google Sheets
- Python (pandas)
- MATLAB
- Any CSV-compatible analysis tool

### Sample Data Format
```csv
Timestamp,Temp_C,Pressure_Pa,Altitude_m,Lat,Lon,GPS_Alt_m,Satellites,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,FlightState
1000,25.5,101325,0,40.7128,-74.0060,0,8,0.1,0.2,9.8,0.01,0.02,0.03,PREFLIGHT
2000,25.6,101320,5,40.7128,-74.0060,5,8,0.2,0.3,9.9,0.02,0.03,0.04,FLIGHT
```

## Troubleshooting

### Common Issues

1. **SD Card Not Initializing**
   - Check SD card format (must be FAT32)
   - Verify CS pin connection
   - Try different SD card

2. **GPS No Fix**
   - Ensure clear view of sky
   - Check baud rate settings
   - Verify RX/TX connections

3. **Temperature Sensor Errors**
   - Check wiring (data, VCC, GND)
   - Verify pull-up resistor
   - Check sensor address

4. **IMU/Barometer Issues**
   - Verify I2C connections
   - Check sensor addresses
   - Ensure proper power supply

### Debug Mode
Enable serial debug output by setting `ENABLE_SERIAL_DEBUG true` in `config.h`.

## Future Enhancements

- [ ] Implement IMU sensor driver
- [ ] Implement barometer sensor driver
- [ ] Implement data compression
- [ ] Add battery voltage monitoring?
- [ ] Add flight event detection (apogee, deployment)

## Contributing

When adding new sensors:
1. Create header file in `include/`
2. Implement driver in `src/`
3. Add initialization to `main.cpp`
4. Update data structure and logging
5. Update documentation

## License

This project is developed for USLI (University Student Launch Initiative) educational purposes.