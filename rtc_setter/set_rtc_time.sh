#!/bin/bash

# RTC Time Setter - Standalone script for rtc_setter directory
# Run this from the rtc_setter directory

echo "=== RTC Time Setter Utility ==="
echo ""
echo "This script will help you set the RTC time on your Arduino Nano"
echo ""

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO not found. Please install PlatformIO first."
    exit 1
fi

# Build the firmware
echo "Building firmware..."
pio run --environment nano_v4

if [ $? -ne 0 ]; then
    echo "✗ Build failed!"
    exit 1
fi

echo ""
echo "✓ Build successful!"
echo ""
echo "Firmware ready at: .pio/build/nano_v4/firmware.hex"
echo ""

# Try to upload
echo "Attempting to upload..."
echo "If upload fails, try power-cycling your Arduino and pressing the reset button"
echo ""

pio run --environment nano_v4 --target upload

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Upload successful!"
    echo ""
    echo "Opening serial monitor..."
    echo "Enter time in format: YYYY MM DD HH MM SS"
    echo "Example: 2025 1 15 14 30 0"
    echo ""
    
    # Wait a moment for the device to boot
    sleep 2
    
    # Open serial monitor
    pio device monitor --environment nano_v4
else
    echo ""
    echo "✗ Upload failed! Possible issues:"
    echo "  1. Arduino not in bootloader mode - try pressing reset button"
    echo "  2. Wrong serial port - check connection"
    echo "  3. Try manual upload with Arduino IDE"
    echo ""
    echo "Firmware hex file is ready at: .pio/build/nano_v4/firmware.hex"
    echo "You can upload this manually using Arduino IDE or avrdude"
fi
