# Wiring Diagram - BINSAI System

## Power Management Diagram
POWER BANK (10000mAh Dual USB)
├── USB 1 (5V/2A) → Micro USB → ESP32 VIN
│ └── ESP32 3.3V → Sensors (MQ-135, GPS)
│
└── USB 2 (5V/2A) → USB to DC Jack → LM2596 Step-down (5V→4V)
└── 4V Output → SIM800L VCC
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
| GPIO2 | Onboard LED | - | Status indicator |
| GPIO0 | Button | - | Boot button (pullup) |

## Voltage Divider Circuits

### 1. For HC-SR04 Echo (5V → 3.3V):
Echo (5V) ──┬── 1kΩ ──── ESP32 GPIO18
│
2kΩ
│
GND

### 2. For MQ-135 (if 5V output):
MQ-135 AOUT ──┬── 1kΩ ──── ESP32 GPIO34
│
1kΩ
│
GND

## Power Consumption Estimate

| Component | Voltage | Current (max) | Power |
|-----------|---------|---------------|-------|
| ESP32 | 5V | 500mA | 2.5W |
| SIM800L (transmit) | 4V | 2A | 8W |
| GPS NEO-6M | 3.3V | 45mA | 0.15W |
| HC-SR04 | 5V | 15mA | 0.075W |
| MQ-135 | 5V | 150mA | 0.75W |
| LCD I2C | 5V | 20mA | 0.1W |
| **TOTAL** | - | **~2.73A peak** | **~11.6W peak** |

## Safety Notes
1. **Separate power for SIM800L** is MANDATORY to prevent ESP32 brownout
2. Always use **common ground** between all components
3. **Voltage dividers** are required for 5V signals to ESP32
4. **Power bank** must provide stable 2A+ per USB port
5. **Fuse protection** recommended for SIM800L circuit (1A fast-blow)