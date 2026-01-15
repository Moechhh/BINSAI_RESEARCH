# Technical Bill of Materials (BOM) - Project BINSAI
**IoT-Integrated Smart Waste Management Monitoring System**

This document provides a comprehensive breakdown of the hardware components, technical specifications, and budgetary estimations required for the construction of a single BINSAI prototype unit.

| No | Core Component | Technical Specifications | Qty | Unit Price (IDR) | Total (IDR) |
|:--:|:---|:---|:--:|:---:|:---:|
| **1** | **Microcontroller Unit (MCU)** | ESP32 DevKit V1 (Dual-core, WiFi + Bluetooth) | 1 | Rp 55,000 | Rp 55,000 |
| **2** | **Ultrasonic Distance Sensor** | HC-SR04 (Non-contact, 2cm - 400cm range) | 1 | Rp 12,000 | Rp 12,000 |
| **3** | **Gas/Air Quality Sensor** | MQ-135 (Detection of NH3, Benzene, Alcohol) | 1 | Rp 18,000 | Rp 18,000 |
| **4** | **Cellular Communication Module** | SIM800L GPRS/GSM Module (Mini V2.0) | 1 | Rp 85,000 | Rp 85,000 |
| **5** | **Global Positioning System (GPS)** | NEO-6M Module with Active Ceramic Antenna | 1 | Rp 65,000 | Rp 65,000 |
| **6** | **Primary Power Supply** | Olike Power Bank 10,000mAh (22.5W Fast Charge) | 1 | Rp 199,000 | Rp 199,000 |
| **7** | **Power Interface I** | USB to DC 5V 2.1mm/Header Cable (MCU Dedicated) | 1 | Rp 15,000 | Rp 15,000 |
| **8** | **Power Interface II** | USB to DC 5V 2.1mm/Header Cable (GSM Dedicated) | 1 | Rp 15,000 | Rp 15,000 |
| **9** | **Industrial Enclosure** | IP-rated Waterproof Box / Custom 3D Printed Chassis | 1 | Rp 35,000 | Rp 35,000 |
| **10**| **Network Connectivity** | SIM Card (Cellular Data & SMS Provisioning) | 1 | Rp 15,000 | Rp 15,000 |
| **Total Estimated Expenditure** | | | | **Rp 514,000** |

---

## Technical Justification & Power Infrastructure Analysis

### 1. Microarchitecture Selection (ESP32)
The **ESP32** was selected due to its dual-core processing capabilities, which allow for simultaneous sensor data acquisition and complex communication task handling. Its integrated low-power modes are essential for optimizing the device's operational lifespan in field deployments.

### 2. Telemetry and Environmental Sensing
* **HC-SR04:** Utilized for non-contact volumetric measurement, ensuring sensor longevity by avoiding direct exposure to waste contaminants.
* **MQ-135:** Employed to monitor ammonia (NH3) concentrations as a biochemical indicator of organic waste decomposition, providing qualitative data beyond physical volume.

### 3. Power Distribution Strategy
To ensure system stability, the device employs a **dual-channel power distribution** method via two independent USB-to-DC interfaces. This design choice is critical to:
* **Current Isolation:** Preventing the high-current bursts (up to 2A) typical of the SIM800L module from causing voltage drops that could trigger MCU brownout resets.
* **Ease of Maintenance:** The use of a high-capacity commercial power bank (Olike 22.5W) ensures long-term uptime and simplifies the recharging process for municipal waste management personnel.

---
*Note: Pricing is based on average Indonesian marketplace rates as of January 2026 and is subject to regional market fluctuations.*
