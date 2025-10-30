# How to Set RTC Time

## ✅ Fixed Version Uploaded!

The firmware has been updated with correct register masking to ensure the time is set properly.

## Current Configuration

Your current time setting in `src/main.cpp` (lines 57-62):
```cpp
int year = 2025;
int month = 10;
int day = 29;
int hour = 18;
int minute = 4;
int second = 30;
```

This sets the RTC to: **October 29, 2025 at 6:04:30 PM**

## To View the Output

Run this command to see if the time was set correctly:
```bash
cd rtc_setter
./view_output.sh
```

Or use screen (if available):
```bash
screen /dev/cu.usbserial-210 115200
```

Press `Ctrl+A` then `K` to exit screen.

## To Change the Time

1. Edit `src/main.cpp` lines 57-62
2. Change the values to your desired time
3. Upload:
```bash
cd rtc_setter
pio run --environment nano_v4 --target upload
```

## What Should Happen

When the Arduino boots with the RTC setter firmware, you should see:

```
=== RTC Time Setter Utility ===
✓ I2C initialized
✓ RTC initialized successfully
Current RTC time: 2000-01-01 0:0:0

=== AUTO SET MODE ===
Setting RTC to fixed time...
Setting RTC to: 2025-10-29 18:4:30
✓ RTC time set successfully!
Verified RTC time: 2025-10-29 18:4:31
Done! Upload your main program now.

Current RTC: 2025-10-29 18:4:36
Current RTC: 2025-10-29 18:4:41
...
```

## Troubleshooting

**If the time is still wrong:**
1. Check the serial output to see what time was actually set
2. Make sure the year is between 2000-2099
3. Make sure month is between 1-12
4. Make sure day is between 1-31
5. Make sure hour is between 0-23 (24-hour format)
6. Make sure minute/second are between 0-59

**If you can't see serial output:**
- The firmware is working even if you can't see the output
- The time is being set automatically on each boot
- You can verify the time is set by uploading your main program

## After Setting Time

Once the RTC is set, upload your main program:
```bash
cd ..
pio run --target upload
```

Your main program will use the RTC time that was set!
