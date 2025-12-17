#!/bin/bash

# Button Test - Quick Run Script
# This script compiles, uploads, and monitors the button test

# Configuration
BOARD="arduino:avr:nano"
BAUD_RATE="115200"
TEST_DIR="examples/button_test"
BUILD_DIR="examples/button_test/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}=== Button Test - Run Script ===${NC}"
}

# Get project root (script is in examples/button_test/)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/../.." && pwd )"

cd "$PROJECT_ROOT" || exit 1

print_header

# Check if arduino-cli is installed
if ! command -v arduino-cli &> /dev/null; then
    print_error "arduino-cli is not installed!"
    echo "Please install it from: https://arduino.github.io/arduino-cli/"
    exit 1
fi

# Detect Arduino port
print_status "Detecting Arduino port..."
ARDUINO_PORT=$(arduino-cli board list | grep "usbserial" | awk '{print $1}' | head -1)

if [ -z "$ARDUINO_PORT" ]; then
    print_error "No Arduino found!"
    echo ""
    echo "Please:"
    echo "  1. Connect your Arduino Nano to USB"
    echo "  2. Wait a few seconds"
    echo "  3. Run: arduino-cli board list"
    echo "  4. Then run this script again"
    exit 1
fi

print_status "Found Arduino at: $ARDUINO_PORT"
echo ""

# Compile
print_status "Compiling button test..."
arduino-cli compile --fqbn $BOARD --output-dir "$BUILD_DIR" "$TEST_DIR"

if [ $? -ne 0 ]; then
    print_error "Compilation failed!"
    exit 1
fi

print_status "Compilation successful!"
echo ""

# Upload
print_status "Uploading to Arduino..."
arduino-cli upload --fqbn $BOARD --port $ARDUINO_PORT --input-dir "$BUILD_DIR"

if [ $? -ne 0 ]; then
    print_error "Upload failed!"
    exit 1
fi

print_status "Upload successful!"
echo ""

# Monitor
print_status "Starting serial monitor..."
print_status "Press Ctrl+C to stop monitoring"
echo ""
echo "----------------------------------------"
echo "Button Test Output:"
echo "----------------------------------------"
echo ""

arduino-cli monitor --port $ARDUINO_PORT --config baudrate=$BAUD_RATE

