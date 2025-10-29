#!/bin/bash

# RTC Time Setter Script
# This script uploads the RTC time setter utility and opens the serial monitor

echo "=== RTC Time Setter Utility ==="
echo "This will upload the RTC time setter and open the serial monitor."
echo "Follow the prompts to set your RTC time."
echo ""

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO not found. Please install PlatformIO first."
    exit 1
fi

# Check if rtc_setter directory exists
if [ ! -d "rtc_setter" ]; then
    echo "Error: rtc_setter directory not found!"
    echo "Please run this script from the project root directory."
    exit 1
fi

echo "Uploading RTC time setter..."
pio run --project-dir rtc_setter --environment nano_v4 --target upload

# Check if upload was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Upload successful!"
    echo "Opening serial monitor..."
    echo "Enter time in format: YYYY MM DD HH MM SS"
    echo "Example: 2025 10 14 21 30 0"
    echo ""
    pio device monitor --project-dir rtc_setter --environment nano_v4
else
    echo "✗ Upload failed!"
    exit 1
fi

