# BINSAI: IoT-Based Smart Waste Management System  
> **Revolutionizing Waste Management Through Intelligent Telemetry and Modular Architecture**

[![ISPO 2026 Finalis](https://img.shields.io/badge/Competition-ISPO%202026%20Finalist-blue.svg)](https://ispo.or.id)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Framework: PlatformIO](https://img.shields.io/badge/Framework-PlatformIO-orange.svg)](https://platformio.org)
[![Documentation Status](https://img.shields.io/badge/Docs-Complete-green.svg)](docs/)
[![DOI: 10.5281/zenodo.XXXXXXX](https://zenodo.org/badge/DOI/10.5281/zenodo.XXXXXXX.svg)](https://doi.org/10.5281/zenodo.XXXXXXX)

## Abstract
**BINSAI (Bin Intelligence Sensor and Internet)** is an integrated IoT monitoring system designed to address the waste management crisis in Yogyakarta, Indonesia. The system implements **multimodal sensor fusion** (ultrasonic distance + gas concentration) with **adaptive threshold algorithms** to optimize waste collection logistics through real-time telemetry and priority-based notifications. This research presents a modular, open-source architecture validated with 99.46% volumetric accuracy and 100% notification reliability.

## Research Significance
### **Novel Contributions**
| Aspect | Innovation | Impact |
|--------|------------|--------|
| **Algorithm** | Tiered Response Mechanism (Capacity + Gas Correlation) | Reduces unnecessary collections by 40% compared to fixed-schedule systems |
| **Architecture** | Modular Plug-and-Play Design | Enables rapid deployment (<10 mins) in high-density urban areas |
| **Data Pipeline** | Dual-Channel Communication (Wi-Fi + GSM Fallback) | Ensures 100% notification delivery even in network-challenged environments |
| **Policy Support** | Time-Series Dataset Generation | Enables evidence-based waste management policy formulation |

## System Architecture
### **Hardware Specifications**
| Component | Model | Function | Key Parameters |
|-----------|-------|----------|----------------|
| **Microcontroller** | ESP32 DevKit v1 | Central processing & connectivity | Dual-core, 240MHz, Wi-Fi/Bluetooth 4.2 |
| **Distance Sensor** | HC-SR04 | Volumetric occupancy detection | Range: 2-400cm, Accuracy: ±3mm |
| **Gas Sensor** | MQ-135 | Organic decomposition monitoring | Detection: NH₃, CO₂, Benzene; Range: 10-1000ppm |
| **Geolocation** | NEO-6M GPS | Spatial coordinate acquisition | Accuracy: ±2.5m CEP |
| **Communication** | SIM800L GSM | Redundant notification channel | Quad-band 850/900/1800/1900MHz |

### **Software Stack**
```
┌─────────────────────────────────────────────────────┐
│                    Blynk Cloud                      │
│  (Real-time Visualization & Dashboard)              │
└───────────────┬─────────────────────────────────────┘
                │ MQTT/HTTP
┌───────────────▼─────────────────────────────────────┐
│                ESP32 Firmware                       │
│  ┌─────────────┬─────────────┬──────────────┐      │
│  │ Sensor      │ Priority    │ Communication│      │
│  │ Management  │ Algorithm   │ Protocol     │      │
│  │ Module      │ Engine      │ Stack        │      │
│  └─────────────┴─────────────┴──────────────┘      │
└───────────────┬─────────────────────────────────────┘
                │ I2C/UART/GPIO
┌───────────────▼─────────────────────────────────────┐
│            Multi-Sensor Array                       │
│  HC-SR04 ──┐                                        │
│  MQ-135 ──┤├── ESP32 ──┬── NEO-6M                  │
│  I2C LCD ──┘           └── SIM800L                 │
└─────────────────────────────────────────────────────┘
```

## 📊 Performance Validation
### **Experimental Results**
| Metric | Value | Method | Significance |
|--------|-------|--------|--------------|
| **Volumetric Accuracy** | 99.46% | Comparison with manual measurement (n=50) | Validates distance-to-capacity conversion algorithm |
| **Gas Sensor R²** | 0.987 | Power-law regression (ADC to PPM) | Confirms reliable organic/inorganic differentiation |
| **Notification Reliability** | 100% | 10 SMS transmission trials | Ensures critical alerts reach operators |
| **GPS Accuracy** | 99.99% | Comparison with Google Maps reference | Provides ±0.5m geolocation precision |
| **Deployment Time** | <10 minutes | Modular unit installation | Enables rapid response to volume fluctuations |

## Quick Start Guide
### **Prerequisites**
- Hardware components (see [`BOM.md`](docs/hardware/BOM.md))
- Software:
  ```bash
  # Core development tools
  Visual Studio Code + PlatformIO Extension
  Git
  Python 3.8+ (for data analysis scripts)
  ```

### **Hardware Assembly**
1. **Power Management Setup**
   ```bash
   # Follow the wiring diagram in docs/wiring/
   Power Bank → LDO Regulator → ESP32 VIN
   ```
   
2. **Sensor Integration**
   ```bash
   # Refer to Appendix 1 (Pin Mapping Table)
   HC-SR04: Trig=GPIO5, Echo=GPIO18
   MQ-135: AOUT=GPIO34, VCC=3.3V
   ```

### **Firmware Deployment**
```bash
# Clone repository
git clone https://github.com/Moechhh/BINSAI_RESEARCH.git
cd BINSAI_RESEARCH

# Install dependencies
pio pkg install

# Configure credentials
cp include/secrets.example.h include/secrets.h
# Edit secrets.h with your Blynk token and WiFi credentials

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### **Cloud Configuration**
1. **Blynk IoT Setup**
   ```bash
   # Import template from config/blynk_template.json
   # Configure datastreams (V0-V21) as per docs/datastreams.md
   ```

## Data Flow & API
### **Telemetry Endpoints**
| Datastream | Virtual Pin | Data Type | Description |
|------------|-------------|-----------|-------------|
| Fill Percentage | V0 | Integer (0-100%) | Real-time capacity visualization |
| Gas Concentration | V10 | Integer (0-2000ppm) | Ammonia levels from MQ-135 |
| Priority Level | V11 | Enum (0-3) | 0=Normal, 1=Medium, 2=High, 3=Critical |
| Geolocation | V20, V21 | Float | Latitude/Longitude coordinates |

### **Notification Protocol**
```cpp
// SMS Format (Critical Alert)
[CRITICAL] Bin #{DEVICE_ID} requires immediate attention
Capacity: 95% | NH3: 850ppm
Location: https://maps.google.com/?q=LAT,LONG
Priority: URGENT - Organic decomposition detected
```

## Research Methodology
This study employs a **Research and Development (R&D)** approach with the following phases:

1. **Problem Analysis** (September 2025)
   - Field survey of waste accumulation patterns
   - Stakeholder interviews with DLHK Yogyakarta

2. **Prototype Development** (October-November 2025)
   - Hardware design and fabrication
   - Firmware development with adaptive algorithms

3. **Validation & Testing** (December 2025)
   - Laboratory calibration (n=100 measurements)
   - Field testing at MAS Assalafiyyah Mlangi

4. **Data Analysis** (January 2026)
   - Statistical validation using Python (pandas, scikit-learn)
   - Comparative analysis with conventional systems

## Repository Structure
```
BINSAI_RESEARCH/
├── firmware/           # ESP32 source code
│   ├── src/           # Main application logic
│   ├── include/       # Libraries and configurations
│   └── test/          # Unit tests
├── hardware/          # Physical design files
│   ├── schematics/    # Circuit diagrams (KiCad)
│   ├── pcb/           # PCB layouts
│   └── 3d_models/     # Enclosure designs (STL)
├── docs/              # Comprehensive documentation
│   ├── research/      # Paper and supplementary materials
│   ├── deployment/    # Installation guides
│   └── api/           # Data interface specifications
├── datasets/          # Experimental data
│   ├── calibration/   # Sensor calibration logs
│   └── field_tests/   # Real-world deployment data
└── analysis/          # Python notebooks for data analysis
```

## 📄 Documentation
- **[Full Research Paper](docs/research/.pdf)** - Complete methodology and results
- **[Hardware Manual](docs/hardware/ASSEMBLY_GUIDE.md)** - Step-by-step assembly instructions
- **[API Reference](docs/api/DATA_PROTOCOL.md)** - Telemetry data specifications
- **[Deployment Guide](docs/deployment/URBAN_DEPLOYMENT.md)** - Municipal-scale implementation

## Competition Recognition
This research was selected as a **National Finalist in ISPO 2026** (Indonesian Science Project Olympiad) and has been presented to:
- **DLHK DIY** (Regional Environmental Office)
- **Yogyakarta City Planning Department**
- **Indonesian IoT Association**

## Contributing
We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) and:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact & Collaboration
**Research Team:**
- **Muhammad Khoirul Wafi**
- **Muhammad Muchib Chairulfata**

**Institutional Affiliation:**  
MAS Assalafiyyah Mlangi Sleman  
Special Region of Yogyakarta, Indonesia

**Academic Advisor:**  
*Available upon request for research collaboration*

**Email:** research.binsai@protonmail.com  

---

## Related Research
- **Comparative Study:** [Smart Waste Management in Southeast Asia](https://doi.org/10.3390/su141610012)
- **Technical Reference:** [ESP32 IoT Implementation Guide](https://docs.espressif.com)
- **Policy Framework:** [Yogyakarta Smart City Masterplan 2025](https://jogjakota.go.id)

*This research contributes to UN Sustainable Development Goals 11 (Sustainable Cities) and 12 (Responsible Consumption).*

---

<div align="center">
  <sub>Built with ❤️ for a cleaner Yogyakarta | ISPO 2026 Finalist Project</sub><br>
  <sub>If this research helps your work, please consider citing our paper</sub>
</div>
