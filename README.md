# BINSAI: Smart Waste Monitoring System
## For ISPO Research Competition 2024

### 📋 Project Overview
BINSAI (Bin Intelligence Sensor and Internet) is an IoT-based smart waste monitoring system designed for Yogyakarta City's waste management optimization.

### 🎯 Research Objectives
1. Develop real-time waste bin monitoring system
2. Implement waste classification using MQ-135 sensor
3. Create priority-based notification system with GPS integration
4. Design modular system for easy deployment in tourist areas

### 🛠️ Hardware Requirements
| Component | Specification | Quantity |
|-----------|---------------|----------|
| Microcontroller | ESP32 DevKit v1 | 1 |
| Distance Sensor | HC-SR04 Ultrasonic | 1 |
| Gas Sensor | MQ-135 | 1 |
| GPS Module | NEO-6M | 1 |
| GSM Module | SIM800L | 1 |
| LCD Display | 16x2 I2C (0x27) | 1 |
| Power Supply | Dual USB Power Bank 10000mAh | 1 |

### 🔌 Wiring Diagram
See [docs/wiring_diagram.md](docs/wiring_diagram.md)

### 📱 Blynk Dashboard Configuration
1. Create new template in Blynk IoT
2. Add datastreams according to virtual pins:
   - V0: Fill Percentage (0-100%)
   - V1-V4: LED Indicators
   - V5: Distance (cm)
   - V6: Status
   - V10: PPM Value
   - V11: Priority Level (0-3)
   - V12: Waste Type
   - V13: Recommendation
   - V20: Latitude
   - V21: Longitude

### 🚀 Setup Instructions

#### 1. Clone Repository
```bash
git clone https://github.com/yourusername/BINSAI-Research.git
cd BINSAI-Research