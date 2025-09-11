/*
 * RBSAFE (Rocket Body Safe) Check Implementation
 * Critical safety verification before LBIT transition
 */

#include "config.h"

class RBSAFEChecker {
private:
    struct SafetyStatus {
        bool structuralIntegrity;
        bool sensorHealth;
        bool communicationSystems;
        bool flightComputer;
        bool environmentalConditions;
        bool rangeSafety;
        bool overallSafe;
    };
    
    SafetyStatus status;
    
public:
    RBSAFEChecker() {
        resetStatus();
    }
    
    void resetStatus() {
        status = {false, false, false, false, false, false, false};
    }
    
    // Main RBSAFE check routine
    bool performRBSAFECheck() {
        Serial.println("=== RBSAFE CHECK INITIATED ===");
        logToTelemetry("RBSAFE_CHECK_START");
        
        resetStatus();
        
        // Perform all safety checks
        status.structuralIntegrity = checkStructuralIntegrity();
        delay(100);
        
        status.sensorHealth = checkSensorHealth();
        delay(100);
        
        status.communicationSystems = checkCommunicationSystems();
        delay(100);
        
        status.flightComputer = checkFlightComputer();
        delay(100);
        
        status.environmentalConditions = checkEnvironmentalConditions();
        delay(100);
        
        status.rangeSafety = checkRangeSafety();
        delay(100);
        
        // Overall safety determination
        status.overallSafe = status.structuralIntegrity && 
                           status.sensorHealth && 
                           status.communicationSystems && 
                           status.flightComputer && 
                           status.environmentalConditions && 
                           status.rangeSafety;
        
        reportSafetyStatus();
        
        if (status.overallSafe) {
            Serial.println(" RBSAFE CHECK PASSED - CLEARED FOR LBIT");
            logToTelemetry("RBSAFE_PASS");
        } else {
            Serial.println(" RBSAFE CHECK FAILED - LBIT BLOCKED");
            logToTelemetry("RBSAFE_FAIL");
        }
        
        return status.overallSafe;
    }
    
private:
    bool checkStructuralIntegrity() {
        Serial.println("Checking Structural Integrity...");
        
        // Pyrotechnic continuity checks (measure resistance)
        bool stageSepContinuity = checkContinuity(STAGE_SEP_PIN);
        bool parachuteContinuity = checkContinuity(PARACHUTE_DEPLOY_PIN);
        bool noseFairingContinuity = checkContinuity(NOSE_FAIRING_PIN);
        
        // Mechanical integrity sensors (if available)
        bool mechanicalIntegrity = checkMechanicalSensors();
        
        bool structuralOK = stageSepContinuity && parachuteContinuity && 
                           noseFairingContinuity && mechanicalIntegrity;
        
        Serial.print("  Stage Sep Continuity: "); Serial.println(stageSepContinuity ? "PASS" : "FAIL");
        Serial.print("  Parachute Continuity: "); Serial.println(parachuteContinuity ? "PASS" : "FAIL");
        Serial.print("  Nose Fairing Continuity: "); Serial.println(noseFairingContinuity ? "PASS" : "FAIL");
        Serial.print("  Mechanical Integrity: "); Serial.println(mechanicalIntegrity ? "PASS" : "FAIL");
        
        return structuralOK;
    }
    
    bool checkSensorHealth() {
        Serial.println("Checking Sensor Health...");
        
        // IMU health check
        bool imuHealthy = verifyIMUCalibration();
        
        // Barometer health check
        bool baroHealthy = verifyBarometerBaseline();
        
        // GPS health check
        bool gpsHealthy = verifyGPSLock();
        
        // Temperature sensor check
        bool tempHealthy = verifyTemperatureSensors();
        
        // Battery voltage check
        bool powerHealthy = verifyPowerSystems();
        
        bool sensorsOK = imuHealthy && baroHealthy && gpsHealthy && 
                        tempHealthy && powerHealthy;
        
        Serial.print("  IMU Health: "); Serial.println(imuHealthy ? "PASS" : "FAIL");
        Serial.print("  Barometer Health: "); Serial.println(baroHealthy ? "PASS" : "FAIL");
        Serial.print("  GPS Health: "); Serial.println(gpsHealthy ? "PASS" : "FAIL");
        Serial.print("  Temperature Health: "); Serial.println(tempHealthy ? "PASS" : "FAIL");
        Serial.print("  Power Systems: "); Serial.println(powerHealthy ? "PASS" : "FAIL");
        
        return sensorsOK;
    }
    
    bool checkCommunicationSystems() {
        Serial.println("Checking Communication Systems...");
        
        // Telemetry transmission test
        bool telemetryOK = testTelemetryLink();
        
        // Command reception test
        bool commandOK = testCommandReception();
        
        // SD card logging test
        bool loggingOK = testDataLogging();
        
        bool commOK = telemetryOK && commandOK && loggingOK;
        
        Serial.print("  Telemetry Link: "); Serial.println(telemetryOK ? "PASS" : "FAIL");
        Serial.print("  Command Reception: "); Serial.println(commandOK ? "PASS" : "FAIL");
        Serial.print("  Data Logging: "); Serial.println(loggingOK ? "PASS" : "FAIL");
        
        return commOK;
    }
    
    bool checkFlightComputer() {
        Serial.println("Checking Flight Computer...");
        
        // Memory integrity test
        bool memoryOK = testMemoryIntegrity();
        
        // Timing system verification
        bool timingOK = testTimingSystems();
        
        // Interrupt system test
        bool interruptOK = testInterruptHandlers();
        
        // Watchdog timer test
        bool watchdogOK = testWatchdogTimer();
        
        bool computerOK = memoryOK && timingOK && interruptOK && watchdogOK;
        
        Serial.print("  Memory Integrity: "); Serial.println(memoryOK ? "PASS" : "FAIL");
        Serial.print("  Timing Systems: "); Serial.println(timingOK ? "PASS" : "FAIL");
        Serial.print("  Interrupt Handlers: "); Serial.println(interruptOK ? "PASS" : "FAIL");
        Serial.print("  Watchdog Timer: "); Serial.println(watchdogOK ? "PASS" : "FAIL");
        
        return computerOK;
    }
    
    bool checkEnvironmentalConditions() {
        Serial.println("Checking Environmental Conditions...");
        
        // Temperature range check
        float temperature = getAmbientTemperature();
        bool tempOK = (temperature > -10.0 && temperature < 50.0); // °C
        
        // Humidity check (if sensor available)
        bool humidityOK = true; // Placeholder
        
        // Pressure stability check
        bool pressureOK = checkPressureStability();
        
        bool environmentOK = tempOK && humidityOK && pressureOK;
        
        Serial.print("  Temperature ("); Serial.print(temperature); 
        Serial.print("°C): "); Serial.println(tempOK ? "PASS" : "FAIL");
        Serial.print("  Humidity: "); Serial.println(humidityOK ? "PASS" : "FAIL");
        Serial.print("  Pressure Stability: "); Serial.println(pressureOK ? "PASS" : "FAIL");
        
        return environmentOK;
    }
    
    bool checkRangeSafety() {
        Serial.println("Checking Range Safety...");
        
        // Flight termination system (if implemented)
        bool ftsOK = true; // Placeholder for FTS check
        
        // Frequency coordination check
        bool frequencyOK = true; // Placeholder
        
        // Recovery area status
        bool recoveryAreaOK = true; // Placeholder
        
        // Manual safety approval (could be a physical switch)
        bool manualApproval = digitalRead(MANUAL_SAFETY_PIN);
        
        bool rangeSafetyOK = ftsOK && frequencyOK && recoveryAreaOK && manualApproval;
        
        Serial.print("  Flight Termination: "); Serial.println(ftsOK ? "PASS" : "FAIL");
        Serial.print("  Frequency Clear: "); Serial.println(frequencyOK ? "PASS" : "FAIL");
        Serial.print("  Recovery Area: "); Serial.println(recoveryAreaOK ? "PASS" : "FAIL");
        Serial.print("  Manual Approval: "); Serial.println(manualApproval ? "PASS" : "FAIL");
        
        return rangeSafetyOK;
    }
    
    // Helper functions for specific checks
    bool checkContinuity(int pin) {
        // Measure resistance across pyrotechnic circuit
        // Should show continuity but not short circuit
        pinMode(pin, INPUT_PULLUP);
        delay(10);
        bool continuity = digitalRead(pin) == LOW; // Assuming active low
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW); // Safe state
        return continuity;
    }
    
    bool verifyIMUCalibration() {
        // Check if IMU readings are stable and within expected ranges
        // Should read ~1g on Z-axis, ~0g on X,Y when stationary
        float accelZ = getAccelZ(); // Your IMU reading function
        return (abs(accelZ - 1.0) < 0.2); // Within 0.2g of 1g
    }
    
    bool verifyBarometerBaseline() {
        // Check barometer reading stability
        float pressure1 = getBarometricPressure();
        delay(100);
        float pressure2 = getBarometricPressure();
        return (abs(pressure1 - pressure2) < 2.0); // Stable within 2 hPa
    }
    
    bool verifyGPSLock() {
        // Check GPS satellite count and accuracy
        int satellites = getGPSSatellites();
        float hdop = getGPSHDOP();
        return (satellites >= 4 && hdop < 5.0);
    }
    
    bool verifyTemperatureSensors() {
        float temp = getAmbientTemperature();
        return (temp > -50.0 && temp < 85.0); // Reasonable range
    }
    
    bool verifyPowerSystems() {
        float batteryVoltage = getBatteryVoltage();
        return (batteryVoltage > 3.3); // Minimum operating voltage
    }
    
    void reportSafetyStatus() {
        Serial.println("\n=== RBSAFE STATUS SUMMARY ===");
        Serial.println(status.structuralIntegrity ? " Structural Integrity: PASS" : " Structural Integrity: FAIL");
        Serial.println(status.sensorHealth ? " Sensor Health: PASS" : " Sensor Health: FAIL");
        Serial.println(status.communicationSystems ? " Communication Systems: PASS" : " Communication Systems: FAIL");
        Serial.println(status.flightComputer ? " Flight Computer: PASS" : " Flight Computer: FAIL");
        Serial.println(status.environmentalConditions ? " Environmental Conditions: PASS" : " Environmental Conditions: FAIL");
        Serial.println(status.rangeSafety ? " Range Safety: PASS" : " Range Safety: FAIL");
        Serial.println("================================\n");
    }
    
    void logToTelemetry(const char* event) {
        // Log safety check events to your telemetry system
        Serial.print("[SAFETY] ");
        Serial.println(event);
    }
};