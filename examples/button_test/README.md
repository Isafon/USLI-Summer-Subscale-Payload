# Button Test - Standalone Diagnostics

## Purpose

This standalone test script helps diagnose button hardware issues by:
- Showing **continuous raw button readings** (HIGH/LOW states)
- Detecting **button state changes** in real-time
- Testing **debouncing logic** (same as main code)
- Providing **detailed statistics** (press count, timing)
- Offering **visual feedback** via built-in LED

Use this test to verify:
- ✅ Button is wired correctly
- ✅ Pin connections are solid
- ✅ Button hardware is working
- ✅ Pull-up resistor is functioning
- ✅ Button press detection logic works

## Hardware Setup

### Button Wiring
```
Button Configuration:
  - One side of button -> Pin 4 (D4)
  - Other side of button -> GND
  - Internal pull-up resistor (enabled in code)
```

**Important:** The button must be wired to **connect Pin 4 to GND when pressed**.
- When **NOT pressed**: Pin 4 reads HIGH (pulled up internally)
- When **PRESSED**: Pin 4 reads LOW (connected to GND)

### Expected Behavior

| State | Pin 4 Reading | LED (Pin 13) |
|-------|---------------|--------------|
| Button NOT pressed | HIGH (1) | OFF |
| Button PRESSED | LOW (0) | ON |

## Quick Start (Easiest Method)

**Just run this from the project root:**

```bash
./examples/button_test/run_button_test.sh
```

This script will automatically:
1. ✅ Detect your Arduino port
2. ✅ Compile the test
3. ✅ Upload to your Arduino
4. ✅ Start the serial monitor

---

## How to Run (Manual Method)

### Method 1: Using the Helper Script (Recommended)

From the project root:
```bash
./examples/button_test/run_button_test.sh
```

### Method 2: Manual Steps

#### 1. Compile the Test

```bash
cd examples/button_test
arduino-cli compile --fqbn arduino:avr:nano .
```

#### 2. Find Your Arduino Port

```bash
arduino-cli board list
```

Look for a line like:
```
/dev/cu.usbserial-B004808T    serial       Arduino Nano
```

#### 3. Upload to Arduino

Replace `/dev/cu.usbserial-*` with your actual port:
```bash
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
```

#### 4. Monitor Output

```bash
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

## Understanding the Output

### Startup Information

```
========================================
     BUTTON TEST - Standalone
========================================

Testing button on Pin: 4
Mode: INPUT_PULLUP

Expected behavior:
  - HIGH (1) when button NOT pressed
  - LOW (0) when button PRESSED

========================================
Starting continuous monitoring...
Press button to test
========================================

Initial button state: HIGH (NOT PRESSED)
```

### Continuous Readings

The test shows readings in this format:
```
[Time ms] Raw State | Change | Debounced | Stats
```

Example output:
```
[1234] HIGH (1) = NOT PRESSED |          |             | Presses: 0
[2345] LOW  (0) = PRESSED     | *** STATE CHANGED *** | DEBOUNCED PRESS #1 | Presses: 1, Last press: 0ms ago [CHANGE]
[3456] HIGH (1) = NOT PRESSED | *** STATE CHANGED *** |             | Presses: 1, Last press: 1111ms ago [CHANGE]
```

### What Each Column Means

1. **Time ms**: Milliseconds since Arduino started
2. **Raw State**: Actual digital reading from pin (HIGH/LOW and 1/0)
3. **Change**: Shows "*** STATE CHANGED ***" when button state changes
4. **Debounced**: Shows "DEBOUNCED PRESS #N" when a valid button press is detected (after debounce delay)
5. **Stats**: Total press count and time since last press

## Expected Results

### Normal Operation (Button Working)

```
[1000] HIGH (1) = NOT PRESSED |          |             | Presses: 0
[1500] HIGH (1) = NOT PRESSED |          |             | Presses: 0
[2000] LOW  (0) = PRESSED     | *** STATE CHANGED *** | DEBOUNCED PRESS #1 | Presses: 1, Last press: 0ms ago [CHANGE]
[2500] LOW  (0) = PRESSED     |          |             | Presses: 1, Last press: 500ms ago
[3000] HIGH (1) = NOT PRESSED | *** STATE CHANGED *** |             | Presses: 1, Last press: 1000ms ago [CHANGE]
```

**What to look for:**
- ✅ State changes from HIGH to LOW when you press button
- ✅ State changes from LOW to HIGH when you release button
- ✅ "DEBOUNCED PRESS #N" appears when button is pressed
- ✅ Built-in LED (Pin 13) lights up when button is pressed

### Button Not Working - Always HIGH

```
[1000] HIGH (1) = NOT PRESSED |          |             | Presses: 0
[1500] HIGH (1) = NOT PRESSED |          |             | Presses: 0
[2000] HIGH (1) = NOT PRESSED |          |             | Presses: 0
[2500] HIGH (1) = NOT PRESSED |          |             | Presses: 0
```

**Possible causes:**
- ❌ Button not wired to GND (floating pin)
- ❌ Button not connected to Pin 4
- ❌ Button is broken/stuck open
- ❌ Loose connection

**Solutions:**
- Check button wiring (one side to Pin 4, other to GND)
- Test button with multimeter (should show continuity when pressed)
- Try a different button
- Check for loose wires/jumper connections

### Button Not Working - Always LOW

```
[1000] LOW  (0) = PRESSED     |          |             | Presses: 0
[1500] LOW  (0) = PRESSED     |          |             | Presses: 0
[2000] LOW  (0) = PRESSED     |          |             | Presses: 0
```

**Possible causes:**
- ❌ Pin 4 shorted to GND (wire touching GND)
- ❌ Button stuck closed/always pressed
- ❌ Wrong pin used

**Solutions:**
- Check for short circuit between Pin 4 and GND
- Disconnect button - pin should read HIGH if working
- Test button with multimeter (should show open circuit when not pressed)

### Button Intermittent/Noisy

```
[1000] HIGH (1) = NOT PRESSED |          |             | Presses: 0
[1050] LOW  (0) = PRESSED     | *** STATE CHANGED *** |             | Presses: 0
[1100] HIGH (1) = NOT PRESSED | *** STATE CHANGED *** |             | Presses: 0
[1150] LOW  (0) = PRESSED     | *** STATE CHANGED *** |             | Presses: 0
```

**Possible causes:**
- ❌ Loose connection (vibrating wires)
- ❌ Bad button (contacts bouncing)
- ❌ Interference/noise on pin

**Solutions:**
- Tighten all connections
- Use a different button
- Check for loose breadboard connections
- Try a different pin if available

## Visual Indicators

### Built-in LED (Pin 13)
- **LED OFF** = Button NOT pressed (Pin 4 = HIGH)
- **LED ON** = Button PRESSED (Pin 4 = LOW)

Watch the LED while pressing the button - it should turn on/off immediately with button presses.

## Troubleshooting Checklist

### Button Not Detected at All

1. ✅ **Check wiring:**
   - Button one side → Pin 4
   - Button other side → GND
   - No other connections

2. ✅ **Test pin directly:**
   - Disconnect button
   - Connect Pin 4 directly to GND with a wire
   - Should show LOW when connected
   - Should show HIGH when disconnected

3. ✅ **Verify correct pin:**
   - Check that Pin 4 is the right pin for your board
   - Some boards label pins differently

4. ✅ **Check button:**
   - Test button with multimeter
   - Should show continuity when pressed
   - Should show open circuit when not pressed

### Button Detected But Not Debouncing

If you see many rapid state changes:
- This is normal! The debouncing logic should filter these out
- Look for "DEBOUNCED PRESS #N" - that's the actual press detection
- Multiple state changes before debounced press is expected behavior

### Serial Monitor Not Showing Output

1. ✅ **Check baud rate:** Must be 115200
2. ✅ **Check port:** Make sure you're monitoring the correct port
3. ✅ **Wait for upload:** Give Arduino a few seconds after upload
4. ✅ **Check serial cable:** Some USB cables don't support data transfer

## Comparison with Main Code

This test uses the **exact same configuration** as the main code:

| Setting | Main Code | Test Code | Match? |
|---------|-----------|-----------|--------|
| Pin | 4 | 4 | ✅ |
| Mode | INPUT_PULLUP | INPUT_PULLUP | ✅ |
| Debounce Delay | 100ms | 100ms | ✅ |
| Press Detection | LOW when pressed | LOW when pressed | ✅ |

If this test works but main code doesn't, the issue is likely in:
- Other code interfering with button checks
- Timing issues in main loop
- Button check happening at wrong time

## Next Steps

1. **If test works:** Button hardware is fine - check main code logic
2. **If test doesn't work:** Fix hardware issues first
3. **If intermittent:** Check connections and button quality

## Tips

- Press button firmly and hold for 100ms+ to see debounced detection
- Watch the LED for immediate visual feedback
- Try pressing button multiple times to see press count increment
- Leave button not pressed to verify it stays HIGH
- Hold button down to verify it stays LOW

## Quick Test Command

From project root, you can use:
```bash
cd examples/button_test && \
arduino-cli compile --fqbn arduino:avr:nano . && \
arduino-cli upload --fqbn arduino:avr:nano -p $(arduino-cli board list | grep usb | awk '{print $1}') . && \
arduino-cli monitor -p $(arduino-cli board list | grep usb | awk '{print $1}') -c baudrate=115200
```

Replace the port detection part with your actual port if the grep doesn't work.

