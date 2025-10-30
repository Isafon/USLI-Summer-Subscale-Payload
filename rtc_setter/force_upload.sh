#!/bin/bash

# Force upload script that resets the Arduino via DTR line
PORT="/dev/cu.usbserial-210"

echo "=== Force Upload RTC Setter ==="
echo ""
echo "This script will:"
echo "1. Reset the Arduino via DTR"
echo "2. Upload the firmware"
echo "3. Open serial monitor"
echo ""

# Function to toggle DTR to reset Arduino
reset_arduino() {
    echo "Resetting Arduino via DTR..."
    
    # Open serial port and toggle DTR
    # Using stty to set DTR low then high to trigger reset
    exec 3> "$PORT"
    stty -f "$PORT" 1200 cs8 -cstopb -parenb
    sleep 0.1
    stty -f "$PORT" 115200 cs8 -cstopb -parenb
    exec 3>&-
    
    sleep 0.5
    echo "Reset complete"
}

# Try reset and upload
reset_arduino

echo "Uploading firmware..."
pio run --environment nano_v4 --target upload

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Upload successful!"
    echo ""
    echo "Opening serial monitor in 2 seconds..."
    sleep 2
    pio device monitor --environment nano_v4
else
    echo ""
    echo "✗ Upload failed!"
    echo ""
    echo "Alternative methods:"
    echo "1. Press RESET button twice quickly on Arduino"
    echo "2. Try using Arduino IDE"
    echo "3. Check if bootloader is corrupted"
    echo ""
    echo "Hex file location: .pio/build/nano_v4/firmware.hex"
fi
