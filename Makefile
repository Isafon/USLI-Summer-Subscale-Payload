# Arduino Nano v4 Makefile for USLI Payload Project
# Requires arduino-cli to be installed

# Configuration
BOARD = arduino:avr:nano
PORT = /dev/ttyUSB0
BAUD_RATE = 115200
SKETCH_DIR = app
BUILD_DIR = build

# Arduino CLI commands
ARDUINO_CLI = arduino-cli
COMPILE_CMD = $(ARDUINO_CLI) compile --fqbn $(BOARD) --output-dir $(BUILD_DIR)
UPLOAD_CMD = $(ARDUINO_CLI) upload --fqbn $(BOARD) --port $(PORT) --input-dir $(BUILD_DIR)
MONITOR_CMD = $(ARDUINO_CLI) monitor --port $(PORT) --config baudrate=$(BAUD_RATE)

# Default target
all: compile

# Compile the project
compile:
	@echo "Compiling project..."
	$(COMPILE_CMD) $(SKETCH_DIR)
	@echo "Compilation complete!"

# Upload to Arduino
upload: compile
	@echo "Uploading to Arduino Nano v4..."
	$(UPLOAD_CMD)
	@echo "Upload complete!"

# Monitor serial output
monitor:
	@echo "Starting serial monitor..."
	$(MONITOR_CMD)

# Upload and monitor
deploy: upload
	@echo "Starting serial monitor..."
	$(MONITOR_CMD)

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# List available ports
ports:
	@echo "Available ports:"
	$(ARDUINO_CLI) board list

# Install required libraries
install-libs:
	@echo "Installing required libraries..."
	$(ARDUINO_CLI) lib install "TinyGPSPlus"
	$(ARDUINO_CLI) lib install "OneWire"
	$(ARDUINO_CLI) lib install "DallasTemperature"
	$(ARDUINO_CLI) lib install "Adafruit BusIO"
	$(ARDUINO_CLI) lib install "Adafruit Unified Sensor"
	@echo "Libraries installed!"

# Setup project
setup: install-libs
	@echo "Project setup complete!"

# Help
help:
	@echo "Available targets:"
	@echo "  compile    - Compile the project"
	@echo "  upload     - Compile and upload to Arduino"
	@echo "  monitor    - Start serial monitor"
	@echo "  deploy     - Upload and start monitoring"
	@echo "  clean      - Clean build files"
	@echo "  ports      - List available ports"
	@echo "  install-libs - Install required libraries"
	@echo "  setup      - Setup project (install libraries)"
	@echo "  help       - Show this help"

.PHONY: all compile upload monitor deploy clean ports install-libs setup help

