# RTC Time Setter Utility

This utility allows you to set the RTC (Real-Time Clock) time independently from the main program.

## Files

- `rtc_setter/` - Separate directory containing the RTC setter utility
  - `rtc_setter/src/main.cpp` - The main RTC setter program
  - `rtc_setter/src/rtc_pcf8523.cpp` - RTC library (copied from main project)
  - `rtc_setter/include/` - Header files (copied from main project)
  - `rtc_setter/platformio.ini` - PlatformIO configuration for RTC setter
- `set_rtc.sh` - Script to upload and run the utility
- `RTC_SETTER_README.md` - This documentation

## Wiring

Connect the RTC PCF8523 to your Arduino Nano:

```
RTC PCF8523    Arduino Nano
VCC    ----->  3.3V
GND    ----->  GND  
SDA    ----->  A4
SCL    ----->  A5
```

## Usage

### Method 1: Using the Script (Recommended)

```bash
./set_rtc.sh
```

This will:
1. Upload the RTC setter utility
2. Open the serial monitor
3. Guide you through setting the time

### Method 2: Manual Upload

1. Open `src/set_rtc_time.cpp` in PlatformIO
2. Upload to your Arduino Nano
3. Open Serial Monitor at 115200 baud
4. Follow the prompts

## Setting the Time

1. **Format**: Enter time as `YYYY MM DD HH MM SS`
2. **Example**: `2025 10 14 21 30 0` (October 14, 2025 at 9:30 PM)
3. **Press Enter** to set the time

## Features

- ✅ **Validates input** - Checks for valid date/time ranges
- ✅ **Verifies setting** - Confirms the time was set correctly
- ✅ **Shows current time** - Displays RTC time every 5 seconds
- ✅ **Error handling** - Clear error messages for invalid input
- ✅ **Independent** - Works without the main program

## Troubleshooting

**"Failed to initialize RTC!"**
- Check wiring (SDA->A4, SCL->A5, VCC->3.3V, GND->GND)
- Ensure RTC is powered on
- Try power cycling the Arduino

**"Invalid format!"**
- Use exact format: `YYYY MM DD HH MM SS`
- Include spaces between numbers
- Example: `2025 10 14 21 30 0`

**"Failed to set RTC time!"**
- Check I2C communication
- Ensure RTC is properly connected
- Try again with a different time

## After Setting Time

Once you've set the RTC time:
1. Upload your main program (`main.cpp`)
2. The RTC will maintain the correct time
3. No need to set it again unless power is lost for extended periods

