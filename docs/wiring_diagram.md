# Wiring Diagram - BINSAI System

## Power Management Diagram
Battery 9V to modul XL4015 step-down with heat sink
├── PIN Out (+) → ESP32 VIN and 5VIN SIM800L
│ └── ESP32 3.3V → Sensors (MQ-135, HC-SR04, and GPS)
├── PIN Out (-) → GND
└── Common GND for all components

## Pin Connections Table
| ESP32 GPIO | Component | Pin | Note |
|------------|-----------|-----|------|
| GPIO5 | HC-SR04 | TRIG | Output |
| GPIO18 | HC-SR04 | ECHO | Input (with voltage divider) |
| GPIO34 | MQ-135 | AOUT | Analog input only |
| GPIO16 | SIM800L | TX | ESP32 RX2 |
| GPIO17 | SIM800L | RX | ESP32 TX2 |
| GPIO27 | SIM800L | PWRKEY | Power control |
| GPIO26 | SIM800L | STATUS | Optional status check |
| GPIO13 | GPS NEO-6M | TX | ESP32 RX1 |
| GPIO15 | GPS NEO-6M | RX | ESP32 TX1 |
| GPIO21 | LCD I2C | SDA | I2C Data |
| GPIO22 | LCD I2C | SCL | I2C Clock |


## Safety Notes
1. **Separate power for SIM800L** is MANDATORY to prevent ESP32 brownout
2. Always use **common ground** between all components
3. **Voltage dividers** are required for 5V signals to ESP32
4. **Fuse protection** recommended for SIM800L circuit (1A fast-blow)
