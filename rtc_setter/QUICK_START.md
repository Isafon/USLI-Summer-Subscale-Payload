# RTC Setter - Quick Start Guide

## Upload Methods (Try in order)

### Method 1: Python Reset Script (BEST)
```bash
cd rtc_setter
./reset_and_upload.py
```

This automatically:
- Resets Arduino via DTR
- Uploads firmware
- Opens serial monitor

### Method 2: Manual RESET Button
1. **Press RESET button on Arduino**
2. Immediately run:
```bash
cd rtc_setter
pio run --environment nano_v4 --target upload
```

### Method 3: Arduino IDE
1. Open Arduino IDE
2. File → Open → Navigate to `set_rtc_time/set_rtc_time.ino`
3. Tools → Board → Arduino Nano
4. Tools → Processor → ATmega328P (Old Bootloader)
5. Tools → Port → `/dev/cu.usbserial-210`
6. Click Upload

### Method 4: Lower Baud Rate
```bash
cd rtc_setter
# Edit platformio.ini and change upload_speed to 19200
# Then:
pio run --environment nano_v4 --target upload
```

## Using the RTC Setter

Once uploaded and serial monitor opens:

1. **Wait for initialization** - You'll see:
```
=== RTC Time Setter Utility ===
✓ I2C initialized
✓ RTC initialized successfully
Current RTC time: 2000-01-01 0:0:0
```

2. **Enter new time** in format: `YYYY MM DD HH MM SS`
   - Example: `2025 1 15 14 30 0` (Jan 15, 2025 at 2:30 PM)
   - Example: `2025 10 31 23 59 59` (Oct 31, 2025 at 11:59:59 PM)

3. **Verify** - It will confirm the time was set

4. **Done!** - RTC is now set

## Troubleshooting

### "stk500_getsync()" Error
- Arduino not responding to upload
- **Solution**: Try Method 1 (Python reset) or Method 2 (manual reset)

### "Failed to initialize RTC!"
- Check wiring:
  - SDA → A4
  - SCL → A5  
  - VCC → 3.3V
  - GND → GND

### Wrong Serial Port
```bash
# Find correct port:
ls /dev/cu.*

# Update in platformio.ini:
upload_port = /dev/cu.YOUR_PORT_HERE
```

## Quick Commands Reference

```bash
# Upload with reset
./reset_and_upload.py

# Just open serial monitor
pio device monitor --environment nano_v4

# Rebuild
pio run --environment nano_v4

# Clean and rebuild
pio run --environment nano_v4 --target clean
pio run --environment nano_v4
```
