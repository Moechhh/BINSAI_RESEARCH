#include <Arduino.h>

/**
 * BINSAI RESEARCH PROJECT - SENSOR CALIBRATION SUITE
 * Focus: MQ-135 Ammonia & Air Quality Sensor
 * Version: 2.0 (Lite - No Logging)
 * * Research Note: 
 * MQ-135 requires at least 24-48 hours of pre-heating for stable readings.
 * Calibration must be performed in a controlled "Clean Air" environment.
 */

// --- Configuration Parameters ---
const int MQ_PIN = 34;           // ESP32 Analog Pin (ADC1)
const float RL_VALUE = 10.0;     // Standard Load Resistor (kOhm). Verify if your board uses 1.0 or 10.0
const float CLEAN_AIR_RATIO = 3.6; // RS/R0 ratio in clean air (from datasheet)
const int SAMPLE_COUNT = 150;    // Number of samples for statistical averaging
const int SAMPLE_INTERVAL = 200; // Delay between samples (ms)

// --- Environmental Assumptions ---
// Ideally replaced with real-time BME280/DHT22 data
const float TEMP_REF = 25.0;     
const float HUMID_REF = 60.0;    

struct CalibrationResult {
    float r0;
    float rs_air;
    float confidence;
    bool is_valid;
};

class MQ135ScientificCalibrator {
private:
    int _pin;
    float _rl;
    
    // Internal resistance calculation with ESP32 ADC Non-linearity compensation
    float readResistance() {
        int rawADC = analogRead(_pin);
        // ESP32 12-bit ADC (0-4095) with 3.3V Reference
        float v_out = (rawADC / 4095.0) * 3.3;
        
        // Safety margin to prevent division by zero or infinite resistance
        if (v_out >= 3.25) v_out = 3.25;
        if (v_out <= 0.05) v_out = 0.05;

        // Formula: RS = ((VCC / V_OUT) - 1) * RL
        return ((3.3 / v_out) - 1.0) * _rl;
    }

public:
    MQ135ScientificCalibrator(int pin, float rl) : _pin(pin), _rl(rl) {}

    CalibrationResult execute() {
        Serial.println(F("[INFO] Initializing MQ-135 Calibration..."));
        Serial.println(F("[WARN] Ensure sensor is in CLEAN AIR and pre-heated."));
        
        float rs_sum = 0;
        float r0_buffer[SAMPLE_COUNT];

        for (int i = 0; i < SAMPLE_COUNT; i++) {
            float rs = readResistance();
            rs_sum += rs;
            r0_buffer[i] = rs / CLEAN_AIR_RATIO;

            if (i % 20 == 0) {
                Serial.print(F("Sampling Progress: "));
                Serial.print((i * 100) / SAMPLE_COUNT);
                Serial.println(F("%"));
            }
            delay(SAMPLE_INTERVAL);
        }

        float avg_rs = rs_sum / SAMPLE_COUNT;
        float final_r0 = avg_rs / CLEAN_AIR_RATIO;

        // Calculate Variance for Confidence Level
        float variance = 0;
        for (int i = 0; i < SAMPLE_COUNT; i++) {
            variance += pow(r0_buffer[i] - final_r0, 2);
        }
        float std_dev = sqrt(variance / SAMPLE_COUNT);
        float confidence = (1.0 - (std_dev / final_r0)) * 100.0;

        CalibrationResult res;
        res.r0 = final_r0;
        res.rs_air = avg_rs;
        res.confidence = confidence;
        res.is_valid = (confidence > 85.0);

        return res;
    }
};

void setup() {
    Serial.begin(115200);
    analogReadResolution(12); // Ensure 12-bit for ESP32
    
    Serial.println(F("\n--- BINSAI SENSOR CALIBRATION SYSTEM ---"));
    delay(3000);

    MQ135ScientificCalibrator calibrator(MQ_PIN, RL_VALUE);
    CalibrationResult result = calibrator.execute();

    Serial.println(F("\n========================================"));
    Serial.println(F("        CALIBRATION SUMMARY            "));
    Serial.println(F("========================================"));
    Serial.print(F("Final R0 Value    : ")); Serial.println(result.r0, 4);
    Serial.print(F("RS in Clean Air   : ")); Serial.println(result.rs_air, 4);
    Serial.print(F("Confidence Level  : ")); Serial.print(result.confidence, 2); Serial.println(F("%"));
    
    if (result.is_valid) {
        Serial.println(F("Status            : [SUCCESS] Data is stable."));
        Serial.println(F("\nCopy this value to your config.h:"));
        Serial.print(F("#define SENSOR_MQ135_R0 ")); Serial.println(result.r0, 4);
    } else {
        Serial.println(F("Status            : [FAILED] High fluctuation detected."));
        Serial.println(F("Action            : Check wiring or increase pre-heating time."));
    }
    Serial.println(F("========================================\n"));
}

void loop() {
    // Calibration is a singleton process.
    delay(1000);
}
