#!/bin/bash
# Script to run individual test files

if [ $# -eq 0 ]; then
    echo "Usage: ./run_test.sh <test_file>"
    echo "Example: ./run_test.sh examples/baro/baro_test.cpp"
    exit 1
fi

TEST_FILE=$1
SRC_DIR="src"
TEMP_BACKUP_DIR=".src_backup_temp"
MAIN_FILE="${SRC_DIR}/main.cpp"

# Create src directory if it doesn't exist
mkdir -p "${SRC_DIR}"

# Move all existing src files to temporary backup
if [ -d "${SRC_DIR}" ] && [ "$(ls -A ${SRC_DIR})" ]; then
    echo "Backing up existing src files..."
    mkdir -p "${TEMP_BACKUP_DIR}"
    mv ${SRC_DIR}/*.cpp ${TEMP_BACKUP_DIR}/ 2>/dev/null || true
    mv ${SRC_DIR}/*.h ${TEMP_BACKUP_DIR}/ 2>/dev/null || true
fi

# Copy test file to main.cpp
echo "Setting up test file: ${TEST_FILE}"
cp "${TEST_FILE}" "${MAIN_FILE}"

# Build and upload
echo "Building and uploading..."
pio run --target upload

if [ $? -eq 0 ]; then
    # Start monitoring only if upload succeeded
    echo "Starting serial monitor (press Ctrl+C to exit)..."
    pio device monitor
fi

# Restore original src files
echo "Restoring original files..."
rm -f "${MAIN_FILE}"
if [ -d "${TEMP_BACKUP_DIR}" ]; then
    mv ${TEMP_BACKUP_DIR}/*.cpp ${SRC_DIR}/ 2>/dev/null || true
    mv ${TEMP_BACKUP_DIR}/*.h ${SRC_DIR}/ 2>/dev/null || true
    rmdir "${TEMP_BACKUP_DIR}" 2>/dev/null || true
fi

echo "Done!"
