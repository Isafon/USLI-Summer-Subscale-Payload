#!/bin/bash

# Auto-detect Arduino port and update arduino.sh

echo "Detecting Arduino port..."

# Find Arduino port (look for usbserial devices)
ARDUINO_PORT=$(arduino-cli board list | grep "usbserial" | awk '{print $1}' | head -1)

if [ -z "$ARDUINO_PORT" ]; then
    echo "No Arduino found. Please connect your Arduino and try again."
    exit 1
fi

echo "Found Arduino at: $ARDUINO_PORT"

# Update arduino.sh with the detected port
sed -i.bak "s|PORT=\".*\"|PORT=\"$ARDUINO_PORT\"|" arduino.sh

echo "Updated arduino.sh with port: $ARDUINO_PORT"
echo "You can now use: ./arduino.sh deploy"
