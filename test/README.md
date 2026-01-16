# BINSAI System Testing & Calibration

This directory contains comprehensive testing and calibration procedures for the BINSAI IoT waste management system.

## Test Categories

### 1. Unit Tests (Hardware-Independent)
Tests for algorithms and data processing without hardware dependencies.

### 2. Integration Tests
Hardware-specific calibration and validation procedures for each sensor module.

### 3. System Tests
End-to-end testing of the complete BINSAI system.

## Quick Start

```bash
## Run all unit tests
pio test -e unit

## Run specific integration test
pio test -e integration --test=mq135_calibration

## Run full system test
pio test -e system

```

## Calibration Procedures
MQ-135 Gas Sensor Calibration
Purpose: Determine baseline resistance (R₀) in clean air and establish PPM conversion model.

## Files:

`integration/mq135_calibration.cpp` - Full calibration procedure

`scripts/analyze_calibration.py` - Data analysis and model fitting

## Procedure:

Pre-heat sensor for 24 hours in clean air

Run calibration script for 5 minutes

Calculate average R₀ from 300 samples

Validate with known gas concentrations

## Ultrasonic Sensor Validation
Purpose: Validate distance measurement accuracy and characterize blind zones.

## GPS Module Testing
Purpose: Verify satellite acquisition, accuracy, and NMEA parsing.

## GSM Module Verification
Purpose: Test SMS transmission reliability and network connectivity.

## Test Results Format
All tests generate structured JSON output:

json
{
  "test_name": "mq135_calibration",
  "timestamp": "2026-01-16T10:30:00Z",
  "parameters": {
    "samples": 300,
    "duration_seconds": 300
  },
  "results": {
    "r0_mean": 9.85,
    "r0_std": 0.12,
    "calibration_status": "PASS"
  }
}
## Adding New Tests
Create test file in appropriate category folder

Follow naming convention: `test_[component]_[purpose].cpp`

Include proper error handling and data logging

Add to platformio.ini.test configuration

## Troubleshooting
Common Issues:

Sensor not detected: Check wiring and power supply

Calibration failure: Ensure clean air environment

Test timeout: Increase test duration in configuration

## Documentation
Full Calibration Protocol[https://docs/calibration_protocol.md]
Test Case Specifications[https://docs/test_specifications.md]
Troubleshooting Guide[https://docs/troubleshooting.md]
