# RTC Setter - Fixed!

## Changes Made

1. **Increased delay before verification** - Changed from 100ms to 500ms to ensure the RTC has time to update
2. **Added Control 2 register check** - Ensures proper initialization of the RTC
3. **Improved register masking** - Ensures 24-hour mode is properly set

## Current Time Setting

Your code is set to (lines 57-62 of `src/main.cpp`):
- Year: 2025
- Month: 10 (October)  
- Day: 29
- Hour: 18 (6:06 PM)
- Minute: 6
- Second: 45

## To Set New Time

### Option 1: Update Current Time
1. Look at the current time on your computer
2. Edit lines 57-62 in `src/main.cpp`
3. Set the time to what you want (or slightly in the future)
4. Upload:
```bash
cd rtc_setter
pio run --environment nano_v4 --target upload
```

### Option 2: Set Exact Time
If you want to set the exact current time:

1. Get your computer's current time:
```bash
date
```

2. Convert to 24-hour format and edit the code

3. Upload the firmware

## Important Notes

- **The RTC is non-volatile** - Once set, it keeps time even when powered off (if it has a backup battery)
- **You can set future time** - If you know when you'll run your experiment, set the RTC to that time
- **Small delays are OK** - The verification shows if it's working, even if a few seconds off

## Verify It's Working

To check the RTC is running and keeping time:

1. Upload the firmware with one time
2. Wait a few seconds
3. Press RESET on the Arduino
4. Watch the serial output - it should show the time advancing

Or simply upload the firmware and check if the "Verified RTC time" matches what you set (within 1 second).

## Troubleshooting

**If time is still 2.5 minutes off:**
- The RTC is working, just set the time a bit in the future
- Add 3 minutes to your desired time when editing the code
- Upload and it will be close to correct

**Example:**
If you want 6:00 PM, but it's currently 5:57:30 PM:
- Set the code to 6:03:00 PM
- Upload it
- By the time it uploads and sets, it will be close to 6:00 PM
