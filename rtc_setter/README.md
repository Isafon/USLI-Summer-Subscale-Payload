# RTC Time Setter

## Simple Method: Edit Code & Upload

The firmware now sets the RTC automatically when it boots - no serial interaction needed!

### To Set the Time:

1. **Edit the time values** in `src/main.cpp` (lines 57-62):


2. **Upload to Arduino:**
```bash
cd rtc_setter
pio run --environment nano_v4 --target upload
```

3. **Done!** The RTC is now set to your specified time.

### To See the Output:

Use any serial monitor tool:
```bash
# Using screen (built-in on macOS)
screen /dev/cu.usbserial-210 115200

# Or use the watch script
./watch_output.sh

# Or Arduino IDE Serial Monitor
# Tools → Serial Monitor (115200 baud)
```

### Example Output:

```
=== RTC Time Setter Utility ===
✓ I2C initialized
✓ RTC initialized successfully
Current RTC time: 2000-01-01 0:0:0

=== AUTO SET MODE ===
Setting RTC to fixed time...
Setting RTC to: 2025-1-15 14:30:0
✓ RTC time set successfully!
Verified RTC time: 2025-1-15 14:30:1
Done! Upload your main program now.

Current RTC: 2025-1-15 14:30:2
Current RTC: 2025-1-15 14:30:7
Current RTC: 2025-1-15 14:30:12
...
```

## Quick Reference

### Edit Time:
- File: `src/main.cpp`
- Lines: 57-62
- Format: `year, month, day, hour, minute, second`

### Upload:
```bash
cd rtc_setter
pio run --environment nano_v4 --target upload
```

### Watch Output:
```bash
./watch_output.sh
# or
screen /dev/cu.usbserial-210 115200
```

## Troubleshooting

**"Failed to initialize RTC!"**
- Check wiring (SDA→A4, SCL→A5, VCC→3.3V, GND→GND)

**Port not found**
- Check: `ls /dev/cu.*`
- Update `platformio.ini` if different

**Upload failing**
- Press RESET button before uploading
- Try running upload command 2-3 times
