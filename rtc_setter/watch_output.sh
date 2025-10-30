#!/bin/bash

# Simple serial monitor using screen or cu
PORT="/dev/cu.usbserial-210"
BAUD=115200

echo "=== RTC Setter Output ==="
echo "Port: $PORT"
echo "Baud: $BAUD"
echo ""
echo "Press Ctrl+A then K to exit"
echo ""

if command -v screen &> /dev/null; then
    screen "$PORT" "$BAUD"
elif command -v cu &> /dev/null; then
    cu -l "$PORT" -s "$BAUD"
else
    echo "Error: Need either 'screen' or 'cu' command"
    echo "Install: brew install screen"
fi
