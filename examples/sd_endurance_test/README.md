# SD Card Endurance Test - 16GB Micro SD

## Purpose

This test determines:
- **How long you can log data** on a 16GB SD card
- **Write speed** of your SD card module
- **Maximum flight duration** at different logging rates
- **Card capacity projections** based on actual write rates

## Hardware Setup

```
SD Card Module -> Arduino Nano
CS   -> Pin 11
MOSI -> Pin 11 (SPI)
MISO -> Pin 12 (SPI)
SCK  -> Pin 13 (SPI)
VCC  -> 5V
GND  -> GND
```

**SD Card:**
- 16GB Micro SD card
- FAT32 formatted
- Class 10 recommended for best speed

## How to Run

```bash
cd examples/sd_endurance_test
arduino-cli compile --fqbn arduino:avr:nano .
arduino-cli upload --fqbn arduino:avr:nano -p /dev/cu.usbserial-* .
arduino-cli monitor -p /dev/cu.usbserial-* -c baudrate=115200
```

## Test Modes

### 1. 1 Hz Logging (Command: `1`)
**Use Case:** Preflight and postflight logging
- 1 sample per second
- Low data rate
- Maximum endurance

**Expected Duration:** ~100+ hours

### 2. 10 Hz Logging (Command: `2`)
**Use Case:** Normal flight logging
- 10 samples per second
- Standard telemetry rate
- Good balance of resolution and duration

**Expected Duration:** ~10+ hours

### 3. 20 Hz Logging (Command: `3`)
**Use Case:** High-speed flight events
- 20 samples per second
- High resolution
- More data storage required

**Expected Duration:** ~5+ hours

### 4. Max Speed Test (Command: `4`)
**Use Case:** Stress test to find maximum write speed
- Writes as fast as possible
- Tests SD card performance limits
- Useful for finding bottlenecks

**Expected Duration:** Fills card quickly

## Commands

| Command | Action |
|---------|--------|
| `1` | Start 1 Hz logging |
| `2` | Start 10 Hz logging |
| `3` | Start 20 Hz logging |
| `4` | Start max speed test |
| `s` | Stop logging |
| `i` | Show card info |
| `d` | Delete test files |
| `h` | Show help menu |

## What Gets Logged

Each record simulates full telemetry data (same as actual flight):

```csv
Timestamp,Temp_C,Pressure_hPa,Altitude_m,GPS_Lat,GPS_Lon,GPS_Alt_m,GPS_Sats,Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,State
```

**Approximate size:** ~150 bytes per record

## Expected Results (16GB Card)

### Data Capacity Estimates

| Rate | Records/sec | MB/hour | Hours to Fill 16GB |
|------|-------------|---------|-------------------|
| 1 Hz | 1 | ~0.5 MB | ~32,000 hours (1,333 days) |
| 10 Hz | 10 | ~5 MB | ~3,200 hours (133 days) |
| 20 Hz | 20 | ~10 MB | ~1,600 hours (67 days) |

*Note: These are theoretical maximums. Actual results will vary based on:*
- SD card write speed
- File system overhead
- Already used space on card

### Typical Rocket Flight Scenarios

**Scenario 1: Standard Flight (300m apogee)**
- Preflight: 5 minutes @ 1 Hz = ~45 KB
- Flight: 60 seconds @ 10 Hz = ~90 KB
- Postflight: 10 minutes @ 1 Hz = ~90 KB
- **Total:** ~225 KB per flight
- **Flights per 16GB:** ~71,000 flights

**Scenario 2: Extended Flight (1000m apogee)**
- Preflight: 10 minutes @ 1 Hz = ~90 KB
- Flight: 180 seconds @ 10 Hz = ~270 KB
- Postflight: 30 minutes @ 1 Hz = ~270 KB
- **Total:** ~630 KB per flight
- **Flights per 16GB:** ~25,000 flights

**Scenario 3: High-Speed Logging**
- Preflight: 5 minutes @ 1 Hz = ~45 KB
- Flight: 60 seconds @ 20 Hz = ~180 KB
- Postflight: 5 minutes @ 1 Hz = ~45 KB
- **Total:** ~270 KB per flight
- **Flights per 16GB:** ~59,000 flights

## Sample Output

```
========================================
SD Card Endurance Test - 16GB
========================================

Initializing SD card... ✓ OK

SD Card Information:
  Card Size: 16000 MB
  Used Space: 150 MB (0.9%)
  Free Space: 15850 MB

Starting logging: 10 Hz (Normal Flight)
File: log_10hz.csv
Rate: 10.0 Hz

✓ Logging started!

Timestamp	Records	Bytes	Write(ms)	Free(MB)	Used%
----------------------------------------------------------------
10s		100	15K	2		15850		0.9%
20s		200	30K	2		15850		0.9%
30s		300	45K	2		15850		0.9%

========================================
Capacity Projection
========================================
Current Write Rate: 1.50 KB/s (5.40 MB/hour)
Free Space: 15850 MB (0.9% used)
Time to Fill Card: 122 days, 15 hours

Maximum Flight Times:
  @ 1 Hz:  105 days, 11 hours
  @ 10 Hz: 10 days, 13 hours
  @ 20 Hz: 5 days, 6 hours
========================================
```

## Interpreting Results

### Write Speed Indicators

**Good Performance:**
- Write time: 1-5ms per record
- Consistent timing (min ≈ max)
- Smooth data rate

**Poor Performance:**
- Write time: >20ms per record
- Highly variable timing
- May indicate:
  - Slow/fake SD card
  - Poor connections
  - Card nearing end of life

### Capacity Projections

The test calculates:
1. **Current write rate** - Based on actual bytes written
2. **Time to fill card** - At current logging rate
3. **Maximum flight times** - At different logging rates

Use these projections to plan:
- How many flights you can log
- When to download/clear data
- Backup schedule

## Tips

1. **Run each test mode** - Different rates may have different performance
2. **Let it run for several minutes** - Gets more accurate projections
3. **Check actual files on computer** - Verify data integrity
4. **Compare different SD cards** - Find the fastest/most reliable
5. **Test with old/new cards** - See performance degradation

## Safety Notes

- Stop test before card is 100% full (test stops at 95%)
- Always backup data before deleting test files
- Format card periodically for best performance
- Use quality SD cards for flight (not fake/counterfeit)

## Quick Math Reference

**16GB = 16,000 MB = 16,384,000 KB**

**Record size:** ~150 bytes
- 1 Hz: 150 bytes/sec = 540 KB/hour = 12.96 MB/day
- 10 Hz: 1,500 bytes/sec = 5.4 MB/hour = 129.6 MB/day
- 20 Hz: 3,000 bytes/sec = 10.8 MB/hour = 259.2 MB/day

**Practical Limits:**
Even at 20 Hz continuous logging, a 16GB card can log for **~62 days** straight!

For rocket flights (typically <5 minutes), you'll **never run out of space**.

## Conclusion

**Bottom line:** With a 16GB card, you have **more than enough space** for hundreds or even thousands of rocket flights. The limiting factor will be **retrieval and analysis**, not storage capacity.

This test helps you:
- ✅ Verify SD card is working properly
- ✅ Measure actual write speeds
- ✅ Confirm you have adequate storage
- ✅ Plan data management strategy
