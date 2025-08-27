#!/bin/bash

# Arduino Test Script for USLI Payload
# Usage: ./test_arduino.sh [test_type]

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}=== Arduino Test Suite ===${NC}"
}

# Configuration
BOARD="arduino:avr:nano"
PORT="/dev/cu.usbserial-B004808T"
BAUD_RATE="115200"
BUILD_DIR="build"

# Test LED blink (simple test)
test_led() {
    print_status "Testing LED blink..."
    
    # Create temporary test directory
    mkdir -p temp_test
    cp tests/led_test.ino temp_test/app.ino
    
    # Build and upload
    arduino-cli compile --fqbn $BOARD --output-dir $BUILD_DIR temp_test
    if [ $? -eq 0 ]; then
        print_status "LED test built successfully"
        arduino-cli upload --fqbn $BOARD --port $PORT --input-dir $BUILD_DIR
        if [ $? -eq 0 ]; then
            print_status "LED test uploaded successfully"
            print_status "Watch the built-in LED (pin 13) - it should blink every second"
            print_status "Starting serial monitor (Ctrl+C to stop)..."
            arduino-cli monitor --port $PORT --config baudrate=$BAUD_RATE
        else
            print_error "LED test upload failed"
        fi
    else
        print_error "LED test build failed"
    fi
    
    # Clean up
    rm -rf temp_test
}

# Test full system (without SD card requirement)
test_full_system() {
    print_status "Testing full telemetry system (test mode)..."
    
    # Create temporary test directory with all required files
    mkdir -p temp_test
    cp tests/test_mode.ino temp_test/app.ino
    cp app/*.h temp_test/
    cp app/*.cpp temp_test/
    cp app/config.h temp_test/
    
    # Build and upload
    arduino-cli compile --fqbn $BOARD --output-dir $BUILD_DIR temp_test
    if [ $? -eq 0 ]; then
        print_status "Full system test built successfully"
        arduino-cli upload --fqbn $BOARD --port $PORT --input-dir $BUILD_DIR
        if [ $? -eq 0 ]; then
            print_status "Full system test uploaded successfully"
            print_status "LED patterns:"
            print_status "  3 blinks = Startup"
            print_status "  5 blinks = Ready"
            print_status "  1 blink per data sample = Normal operation"
            print_status "  10 blinks = Flight detected"
            print_status "  15 blinks = Landing detected"
            print_status "Starting serial monitor (Ctrl+C to stop)..."
            arduino-cli monitor --port $PORT --config baudrate=$BAUD_RATE
        else
            print_error "Full system test upload failed"
        fi
    else
        print_error "Full system test build failed"
    fi
    
    # Clean up
    rm -rf temp_test
}

# Test production system (with SD card)
test_production() {
    print_status "Testing production system (requires SD card)..."
    
    # Copy production sketch to app directory
    cp app/app.ino app/app.ino.backup
    cp app/app.ino app/app.ino
    
    # Build and upload
    arduino-cli compile --fqbn $BOARD --output-dir $BUILD_DIR app
    if [ $? -eq 0 ]; then
        print_status "Production system built successfully"
        arduino-cli upload --fqbn $BOARD --port $PORT --input-dir $BUILD_DIR
        if [ $? -eq 0 ]; then
            print_status "Production system uploaded successfully"
            print_status "Starting serial monitor (Ctrl+C to stop)..."
            arduino-cli monitor --port $PORT --config baudrate=$BAUD_RATE
        else
            print_error "Production system upload failed"
        fi
    else
        print_error "Production system build failed"
    fi
}

# Show help
show_help() {
    print_header
    echo "Usage: $0 [test_type]"
    echo ""
    echo "Test Types:"
    echo "  led         - Simple LED blink test (verifies Arduino is working)"
    echo "  full        - Full telemetry system test (no SD card required)"
    echo "  production  - Production system test (requires SD card)"
    echo "  help        - Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 led       # Test basic Arduino functionality"
    echo "  $0 full      # Test all sensors without SD card"
    echo "  $0 production # Test complete system with SD card"
}

# Main script logic
main() {
    print_header
    
    case "${1:-help}" in
        "led")
            test_led
            ;;
        "full")
            test_full_system
            ;;
        "production")
            test_production
            ;;
        "help"|*)
            show_help
            ;;
    esac
}

# Run main function
main "$@"
