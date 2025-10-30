#!/bin/bash

# Manual upload script for RTC setter
# This attempts to upload the hex file with manual reset trigger

HEX_FILE=".pio/build/nano_v4/firmware.hex"
PORT="/dev/cu.usbserial-210"
BAUD="57600"

echo "=== Manual RTC Setter Upload ==="
echo ""
echo "Ready to upload: $HEX_FILE"
echo "Port: $PORT"
echo ""
echo "INSTRUCTIONS:"
echo "1. Press RESET button on Arduino"
echo "2. This script will attempt upload"
echo "3. Timing is critical - try again if it fails"
echo ""

read -p "Press ENTER when ready, or Ctrl+C to cancel..."

avrdude -C ~/.platformio/packages/tool-avrdude/etc/avrdude.conf \
        -v \
        -p atmega328p \
        -c arduino \
        -P "$PORT" \
        -b "$BAUD" \
        -D \
        -U flash:w:"$HEX_FILE":i

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Upload successful!"
    echo ""
    echo "Opening serial monitor in 3 seconds..."
    sleep 3
    pio device monitor --environment nano_v4
else
    echo ""
    echo "✗ Upload failed!"
    echo ""
    echo "Try again:"
    echo "1. Press reset button on Arduino"
    echo "2. Run this script again immediately"
    echo ""
    echo "Or manually upload the hex file using Arduino IDE"
fi
