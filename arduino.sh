#!/bin/bash

# Arduino Nano v4 Control Script for USLI Payload Project
# Usage: ./arduino.sh [command]

# Configuration
BOARD="arduino:avr:nano"
# Default port (used as a fallback if auto-detection fails)
DEFAULT_PORT="/dev/cu.usbserial-B004808T"
# Allow overriding via environment variable: PORT=/dev/your-port ./arduino.sh upload
PORT="${PORT:-$DEFAULT_PORT}"
BAUD_RATE="115200"
SKETCH_DIR="app"
BUILD_DIR="build"

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

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}=== USLI Payload Arduino Control ===${NC}"
}

# Auto-detect Arduino serial port if needed
resolve_port() {
    # If PORT is set and exists, keep it
    if [ -n "$PORT" ] && [ -e "$PORT" ]; then
        return
    fi

    # Try to auto-detect via arduino-cli
    if command -v arduino-cli &> /dev/null; then
        local detected
        detected=$(arduino-cli board list | awk '/usbserial|ttyUSB|ttyACM/{print $1; exit}')
        if [ -n "$detected" ]; then
            PORT="$detected"
            print_status "Auto-detected Arduino port: $PORT"
            return
        fi
    fi

    print_error "Could not find Arduino serial port."
    echo "  - Make sure the board is plugged in"
    echo "  - Run 'arduino-cli board list' to see available ports"
    echo "  - Then rerun with: PORT=/dev/your-port ./arduino.sh upload"
    exit 1
}

# Check if arduino-cli is installed
check_arduino_cli() {
    if ! command -v arduino-cli &> /dev/null; then
        print_error "arduino-cli is not installed!"
        echo "Please install it from: https://arduino.github.io/arduino-cli/"
        exit 1
    fi
}

# Check if PlatformIO is available
check_platformio() {
    if command -v pio &> /dev/null; then
        PLATFORMIO_AVAILABLE=true
    else
        PLATFORMIO_AVAILABLE=false
    fi
}

# Build using arduino-cli
build_arduino_cli() {
    print_status "Building with arduino-cli..."
    arduino-cli compile --fqbn $BOARD --output-dir $BUILD_DIR $SKETCH_DIR
    if [ $? -eq 0 ]; then
        print_status "Build successful!"
    else
        print_error "Build failed!"
        exit 1
    fi
}

# Build using PlatformIO
build_platformio() {
    print_status "Building with PlatformIO..."
    pio run
    if [ $? -eq 0 ]; then
        print_status "Build successful!"
    else
        print_error "Build failed!"
        exit 1
    fi
}

# Upload using arduino-cli
upload_arduino_cli() {
    resolve_port
    print_status "Uploading with arduino-cli..."
    arduino-cli upload --fqbn $BOARD --port $PORT --input-dir $BUILD_DIR
    if [ $? -eq 0 ]; then
        print_status "Upload successful!"
    else
        print_error "Upload failed!"
        exit 1
    fi
}

# Upload using PlatformIO
upload_platformio() {
    print_status "Uploading with PlatformIO..."
    pio run --target upload
    if [ $? -eq 0 ]; then
        print_status "Upload successful!"
    else
        print_error "Upload failed!"
        exit 1
    fi
}

# Monitor serial output
monitor_serial() {
    resolve_port
    print_status "Starting serial monitor..."
    print_status "Press Ctrl+C to stop monitoring"
    arduino-cli monitor --port $PORT --config baudrate=$BAUD_RATE
}

# List available ports
list_ports() {
    print_status "Available ports:"
    arduino-cli board list
}

# Install libraries
install_libraries() {
    print_status "Installing required libraries..."
    arduino-cli lib install "TinyGPSPlus"
    arduino-cli lib install "OneWire"
    arduino-cli lib install "DallasTemperature"
    arduino-cli lib install "Adafruit BusIO"
    arduino-cli lib install "Adafruit Unified Sensor"
    print_status "Libraries installed!"
}

# Clean build files
clean_build() {
    print_status "Cleaning build files..."
    rm -rf $BUILD_DIR
    if [ "$PLATFORMIO_AVAILABLE" = true ]; then
        pio run --target clean
    fi
    print_status "Clean complete!"
}

# Show help
show_help() {
    print_header
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build     - Build the project"
    echo "  upload    - Build and upload to Arduino"
    echo "  monitor   - Start serial monitor"
    echo "  deploy    - Upload and start monitoring"
    echo "  clean     - Clean build files"
    echo "  ports     - List available ports"
    echo "  libs      - Install required libraries"
    echo "  setup     - Setup project (install libraries)"
    echo "  help      - Show this help"
    echo ""
    echo "Build Systems:"
    echo "  - arduino-cli (default)"
    echo "  - PlatformIO (if available)"
    echo ""
    echo "Configuration:"
    echo "  Board: $BOARD"
    echo "  Port: $PORT"
    echo "  Baud Rate: $BAUD_RATE"
}

# Main script logic
main() {
    print_header
    check_arduino_cli
    check_platformio
    
    case "${1:-help}" in
        "build")
            if [ "$PLATFORMIO_AVAILABLE" = true ]; then
                build_platformio
            else
                build_arduino_cli
            fi
            ;;
        "upload")
            if [ "$PLATFORMIO_AVAILABLE" = true ]; then
                build_platformio
                upload_platformio
            else
                build_arduino_cli
                upload_arduino_cli
            fi
            ;;
        "monitor")
            monitor_serial
            ;;
        "deploy")
            if [ "$PLATFORMIO_AVAILABLE" = true ]; then
                build_platformio
                upload_platformio
            else
                build_arduino_cli
                upload_arduino_cli
            fi
            monitor_serial
            ;;
        "clean")
            clean_build
            ;;
        "ports")
            list_ports
            ;;
        "libs")
            install_libraries
            ;;
        "setup")
            install_libraries
            print_status "Project setup complete!"
            ;;
        "help"|*)
            show_help
            ;;
    esac
}

# Run main function
main "$@"

