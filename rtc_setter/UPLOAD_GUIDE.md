# RTC Setter Upload Guide

## Upload Problems?

The `stk500_getsync()` error means the Arduino isn't responding to the upload command. Try these solutions in order:

### Solution 1: Reset the Arduino
1. Press and release the RESET button on the Arduino
2. Immediately try uploading again (within 2 seconds)
3. Try this 2-3 times if needed

### Solution 2: Check Your Port
Run this to find the correct port:
```bash
pio device list
```

If your port is different, update `platformio.ini`:
```ini
upload_port = /dev/cu.YOUR_PORT_HERE
```

### Solution 3: Manual Upload via Arduino IDE
1. Open Arduino IDE
2. Install PlatformIO hex uploader tool OR
3. Upload the hex file directly:
   - Go to Sketch → Upload Using Programmer
   - Or use avrdude directly (see below)

### Solution 4: Manual Upload via avrdude
The compiled hex file is at: `.pio/build/nano_v4/firmware.hex`

Upload it with:
```bash
avrdude -C ~/.platformio/packages/tool-avrdude/etc/avrdude.conf -v -p atmega328p -c arduino -P /dev/cu.usbserial-210 -b 57600 -D -U flash:w:.pio/build/nano_v4/firmware.hex:i
```

### Solution 5: Bootloader Issues
If nothing works, your bootloader might be corrupted:
1. Use an Arduino as ISP programmer
2. Reflash the bootloader
3. Then try uploading again

## After Successful Upload

Once uploaded, the RTC setter will:
- Initialize on boot
- Show current RTC time
- Wait for you to enter new time
- Validate and set the time
- Verify the setting

### Using the Serial Monitor

Format: `YYYY MM DD HH MM SS`
Example: `2025 1 15 14 30 0` (January 15, 2025 at 2:30 PM)

You'll see:
- Current RTC time displayed
- Prompt to enter new time
- Validation messages
- Confirmation when time is set
- Current time updates every 5 seconds

## Quick Test

To quickly test if upload worked:
```bash
pio device monitor --environment nano_v4 --baud 115200
```

You should see:
```
=== RTC Time Setter Utility ===
This utility will set the RTC time.

✓ I2C initialized
✓ RTC initialized successfully
Current RTC time: YYYY-MM-DD HH:MM:SS

Enter new time in format: YYYY MM DD HH MM SS
Example: 2025 10 14 21 30 0
Press Enter to set time...
```
