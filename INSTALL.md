# Installation Guide - USLI Payload Project

## Quick Start

### Prerequisites
- macOS, Linux, or Windows (WSL recommended for Windows)
- USB cable for Arduino Nano v4
- Formatted SD card (FAT32)

### 1. Install arduino-cli

#### macOS
```bash
brew install arduino-cli
```

#### Linux
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

#### Windows (WSL)
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

### 2. Setup Project
```bash
# Clone the repository (if not already done)
git clone <repository-url>
cd USLI-Summer-Subscale-Payload

# Make the control script executable
chmod +x arduino.sh

# Setup the project (installs libraries)
./arduino.sh setup
```

### 3. Connect Hardware
1. Connect Arduino Nano v4 via USB
2. Connect sensors according to pin configuration
3. Insert SD card

### 4. Build and Upload
```bash
# Deploy (build, upload, and monitor)
./arduino.sh deploy
```

## Alternative: PlatformIO

### Install PlatformIO
```bash
pip install platformio
```

### Build and Upload
```bash
pio run --target upload
pio device monitor
```

## VS Code Setup

### Install Extensions
1. Arduino (Microsoft)
2. PlatformIO IDE (PlatformIO)
3. C/C++ (Microsoft)

### Use Built-in Tasks
1. Open Command Palette (Ctrl+Shift+P)
2. Run `Tasks: Run Task`
3. Select desired Arduino task

## Troubleshooting

### Port Not Found
```bash
# List available ports
./arduino.sh ports

# Update port in arduino.sh if needed
```

### Upload Fails
1. Check USB connection
2. Verify board selection
3. Try different USB cable
4. Check Arduino drivers

### Library Issues
```bash
# Reinstall libraries
./arduino.sh libs
```

## Development Workflow

### Daily Development
```bash
# Make changes to code
# Build to check for errors
./arduino.sh build

# Upload to test
./arduino.sh upload

# Monitor output
./arduino.sh monitor
```

### Before Flight
```bash
# Clean build
./arduino.sh clean

# Full deployment
./arduino.sh deploy

# Verify all sensors working
```

## File Structure
- `app/main.cpp` - Main application
- `app/config.h` - Configuration settings
- `arduino.sh` - Control script
- `platformio.ini` - PlatformIO config
- `Makefile` - Make build system
