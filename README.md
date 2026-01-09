# BINSAI: IoT-Based Smart Waste Management System
> **Revolutionizing Waste, Restoring Yogyakarta, Reshaping Indonesia**

[![ISPO 2026](https://img.shields.io/badge/Competition-ISPO%202026-blue.svg)](https://ispo.or.id)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Framework: PlatformIO](https://img.shields.io/badge/Framework-PlatformIO-orange.svg)](https://platformio.org)

## Executive Summary
**BINSAI (Bin Smart Intelligence and Internet)** is a research-driven IoT solution developed to address the waste crisis in Yogyakarta, specifically focusing on the limited quota of the Piyungan Landfill (TPA). This system implements **priority-based monitoring** and **waste classification** to optimize waste collection logistics through real-time data telemetry.

## Research Significance
Unlike conventional smart bins, BINSAI introduces:
* **Adaptive Priority Algorithm**: Allocates collection resources based on waste criticality (>90% capacity + organic decomposition).
* **Agnostic Infrastructure Architecture**: Designed for future integration with "Yogyakarta Free Wi-Fi" and Smart City ecosystems.
* **Evidence-Based Planning**: Provides time-series data for statistical analysis of urban waste patterns.

## Hardware Architecture
The system utilizes a multi-sensor array for high-fidelity data acquisition:

| Component | Function | Specification |
| :--- | :--- | :--- |
| **ESP32 DevKit v1** | Central Processing Unit | Dual-core, Wi-Fi & BT integrated |
| **HC-SR04** | Volumetric Analysis | Ultrasonic distance sensing |
| **MQ-135** | Decomposition Detection | Ammonia & CO2 (PPM) monitoring |
| **NEO-6M** | Geolocation | GPS Coordinate telemetry |
| **SIM800L** | Redundant Transmission | GSM/GPRS fallback connectivity |

## Software & Data Integration
### IoT Stack
* **Cloud Platform**: Blynk IoT (Datastream V0-V21)
* **Development**: PlatformIO / C++
* **Data Points**: Fill percentage, gas concentration, and priority indexing.

### System Configuration
Refer to the following documentation for detailed setups:
* [**Wiring Diagram**](docs/wiring_diagram.md) - Electrical routing and power management.
* [**Calibration Data**](test/README.md) - Sensor accuracy and threshold validation.

## Getting Started
1. **Prerequisites**: Install [VS Code](https://code.visualstudio.com/) + [PlatformIO IDE](https://platformio.org/platformio-ide).
2. **Installation**:
   ```bash
   git clone [https://github.com/Moechhh/BINSAI_RESEARCH.git](https://github.com/Moechhh/BINSAI_RESEARCH.git)
