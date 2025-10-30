#!/bin/bash

# View RTC setter output using Python
PORT="/dev/cu.usbserial-210"
BAUD=115200

echo "=== RTC Setter Output ==="
echo "Port: $PORT"
echo "Baud: $BAUD"
echo ""
echo "Press Ctrl+C to exit"
echo ""

python3 -c "
import serial
import time

try:
    ser = serial.Serial('$PORT', $BAUD, timeout=1)
    print('Connected! Waiting for output...\n')
    
    while True:
        if ser.in_waiting > 0:
            line = ser.readline()
            print(line.decode('utf-8', errors='ignore'), end='')
        time.sleep(0.1)
except KeyboardInterrupt:
    print('\n\nExiting...')
    ser.close()
except Exception as e:
    print(f'Error: {e}')
"
