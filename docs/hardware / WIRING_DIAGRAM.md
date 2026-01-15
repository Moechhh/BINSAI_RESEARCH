# Hardware Interface & Electrical Specifications - BINSAI

## Power Distribution Network (PDN)
The BINSAI system utilizes a multi-stage power regulation strategy to ensure signal integrity and prevent "brownout" conditions, particularly during high-current GSM transmission bursts.

| Power Rail | Source | Output Voltage | Targeted Sub-systems |
| :--- | :--- | :--- | :--- |
| **Primary (V_in)** | 9V DC Battery | 5.0V (Adjusted) | ESP32 (VIN), SIM800L (VCC)*, I2C LCD |
| **Logic (V_cc)** | Onboard LDO | 3.3V | GPS NEO-6M, MQ-135, HC-SR04 Logic |

> **Technical Note on SIM800L Powering:** > The SIM800L module requires instantaneous current peaks up to 2A during network registration. To accommodate this, the XL4015 Buck Converter is calibrated to **4.0V** and equipped with a dedicated heat sink to optimize thermal dissipation and prevent voltage drops.

## Pin Mapping & Logic Interfacing
This table details the GPIO allocation for the ESP32 DevKit v1, emphasizing the communication protocols used.

| ESP32 GPIO | Module | Pin | Signal Type | Description |
| :--- | :--- | :--- | :--- | :--- |
| **GPIO 5** | HC-SR04 | TRIG | Digital Output | Ultrasonic Trigger Pulse |
| **GPIO 18** | HC-SR04 | ECHO | Digital Input | Pulse Duration (via Voltage Divider) |
| **GPIO 34** | MQ-135 | AOUT | Analog Input | Ammonia Concentration (PPM) |
| **GPIO 13** | GPS NEO-6M | TX | UART RX1 | NMEA Data Stream |
| **GPIO 15** | GPS NEO-6M | RX | UART TX1 | GPS Command Interface |
| **GPIO 16** | SIM800L | TX | UART RX2 | GSM Telemetry Data |
| **GPIO 17** | SIM800L | RX | UART TX2 | AT Command Interface |
| **GPIO 21** | LCD I2C | SDA | I2C Data | Serial Data Line |
| **GPIO 22** | LCD I2C | SCL | I2C Clock | Serial Clock Line |



## Signal Conditioning & System Integrity
To ensure the longevity of the microcontroller and the accuracy of the data, the following electrical safeguards are implemented:

1. **Voltage Logic Translation**: The HC-SR04 Echo signal (5V) is attenuated to 3.3V using a **voltage divider network** (1kΩ and 2kΩ resistors) to remain within the ESP32's GPIO tolerance.
2. **Common Ground Reference**: A unified ground plane is established across all modules to eliminate floating voltages and prevent sensor signal noise.
3. **Electromagnetic Interference (EMI) Shielding**: Data lines for the MQ-135 and GPS are routed away from the GSM antenna to minimize RF interference.

## Safety & Maintenance
* **Thermal Management**: The XL4015 buck converter must be monitored for heat buildup during long-duration operation.
* **Overcurrent Protection**: A 1A fast-blow fuse is recommended for the SIM800L rail in final deployment to prevent component failure.
