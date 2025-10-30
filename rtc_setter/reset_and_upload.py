#!/usr/bin/env python3
"""Reset Arduino and upload firmware"""

import serial
import subprocess
import sys
import time

PORT = "/dev/cu.usbserial-210"

def reset_arduino():
    """Reset Arduino by opening serial at 1200 baud (triggers DTR reset)"""
    print("Resetting Arduino via DTR...")
    try:
        # Opening at 1200 baud triggers bootloader mode on most Arduino bootloaders
        ser = serial.Serial(PORT, 1200)
        ser.close()
        time.sleep(0.5)
        print("✓ Arduino reset")
        return True
    except Exception as e:
        print(f"✗ Reset failed: {e}")
        return False

def upload_firmware():
    """Upload firmware using PlatformIO"""
    print("\nUploading firmware...")
    result = subprocess.run(
        ["pio", "run", "--environment", "nano_v4", "--target", "upload"],
        cwd="/Users/alexbenson/Desktop/USLI-Summer-Subscale-Payload/rtc_setter"
    )
    return result.returncode == 0

def main():
    print("=== RTC Setter Upload ===")
    
    # Reset Arduino
    if not reset_arduino():
        print("\nWarning: Reset may have failed, continuing anyway...")
    
    # Small delay before upload
    time.sleep(0.5)
    
    # Upload firmware
    if upload_firmware():
        print("\n✓ Upload successful!")
        print("\nOpening serial monitor in 2 seconds...")
        time.sleep(2)
        
        # Open serial monitor
        subprocess.run(
            ["pio", "device", "monitor", "--environment", "nano_v4"]
        )
    else:
        print("\n✗ Upload failed!")
        print("\nTry:")
        print("1. Press RESET button on Arduino")
        print("2. Run this script again")
        print("3. Try Arduino IDE")

if __name__ == "__main__":
    main()
