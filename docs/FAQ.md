# BINSAI - Frequently Asked Questions

## **General Questions**

### **What is BINSAI?**
BINSAI (Bin Intelligence Sensor and Internet) is an IoT-based smart waste monitoring system developed to address Yogyakarta's waste management crisis. It integrates ultrasonic sensors, gas sensors, GPS, and GSM modules to provide real-time monitoring and priority-based waste collection.

### **Who developed BINSAI?**
BINSAI was developed by:
- **Muhammad Khoirul Wafi** (Hardware Architecture & System Integration)
- **Muhammad Muchib Chairulfata** (Software Development & IoT Implementation)

From MAS Assalafiyyah Mlangi Sleman, as part of the ISPO 2026 (Indonesian Science Project Olympiad) competition.

### **What problem does BINSAI solve?**
BINSAI addresses:
1. **Landfill overload**: Piyungan landfill accepts only 600 tons/month while Yogyakarta generates 300 tons/day
2. **Inefficient collection**: Fixed-schedule collection regardless of actual bin status
3. **Environmental hazards**: Organic waste decomposition causing health risks
4. **Operational costs**: Unnecessary fuel consumption from empty bin collection

## **Technical Questions**

### **What hardware components are used?**
| Component | Model | Purpose | Cost (IDR) |
|-----------|-------|---------|------------|
| Microcontroller | ESP32 DevKit v1 | Central processing | 85,000 |
| Distance Sensor | HC-SR04 | Volume measurement | 25,000 |
| Gas Sensor | MQ-135 | Decomposition detection | 45,000 |
| GPS Module | NEO-6M | Location tracking | 65,000 |
| GSM Module | SIM800L | SMS notifications | 75,000 |
| Power Bank | 10,000mAh | Portable power | 150,000 |
| **Total** | | | **445,000** |

### **What is the system accuracy?**
- **Volumetric accuracy**: 99.46% (HC-SR04 vs manual measurement)
- **Gas detection validity**: R² = 0.987 (Power-law regression)
- **GPS accuracy**: 99.99% (vs Google Maps reference)
- **Notification reliability**: 100% (10/10 SMS test)

### **How does the priority system work?**
BINSAI uses a **tiered response algorithm** based on two parameters:

```cpp
if (capacity > 90% && gas_ppm > 800) {
    priority = CRITICAL;      // Immediate dispatch
} else if (capacity > 90% || gas_ppm > 450) {
    priority = HIGH;          // Schedule within 6 hours
} else if (capacity > 50%) {
    priority = MEDIUM;        // Schedule within 24 hours
} else {
    priority = NORMAL;        // Routine collection
}
```

### **What is the blind zone issue with HC-SR04?**
The HC-SR04 ultrasonic sensor cannot accurately detect objects within 3cm (blind zone). Our solution:

1. **Software compensation**: Automatically sets status to CRITICAL when distance < 3cm
2. **Hardware adjustment**: 15° angled mounting to minimize impact
3. **Validation**: Infrared proximity sensor as secondary check

### **How long does the battery last?**
- **Normal operation**: 14 days (2-second transmission interval)
- **High-alert mode**: 7 days (1-second transmission during critical conditions)
- **Sleep mode**: 30 days (transmission every 5 minutes)

## **Deployment Questions**

### **How long does installation take?**
- **Target**: 10 minutes per unit
- **Current average**: 12-15 minutes
- **Breakdown**:
  - Physical mounting: 3 minutes
  - GPS satellite lock: 5-8 minutes
  - Network registration: 2-3 minutes
  - System verification: 2 minutes

### **What are the installation requirements?**
1. **Physical space**: Minimum 20×20cm area on bin
2. **Network coverage**: 4G/3G signal strength > -95 dBm
3. **Power source**: Access to charging every 14 days
4. **GPS visibility**: Clear sky view for satellite acquisition

### **Can BINSAI be installed on any trash bin?**
Yes, with these considerations:
- **Size compatibility**: Designed for 50L bins, adjustable for 30-120L
- **Material**: Works on plastic, metal, or concrete bins
- **Location**: Indoor/outdoor compatible (IP65 waterproof rating)
- **Mounting**: Universal bracket system with 3M adhesive or screws

## **Cost & ROI Questions**

### **What is the total cost per unit?**
- **Hardware components**: IDR 445,000
- **Installation labor**: IDR 100,000
- **Monthly operational**: IDR 170,000 (data and maintenance)
- **Total first-year cost**: IDR 2,085,000

### **What is the return on investment (ROI)?**
- **Monthly savings**: IDR 300,000 (fuel, labor, landfill costs)
- **Monthly cost**: IDR 170,000
- **Net monthly benefit**: IDR 130,000
- **Payback period**: 19.5 months
- **Annual ROI**: 61.7%

### **How many units are needed for Yogyakarta?**
- **Pilot phase**: 10-15 units (strategic locations)
- **City coverage**: 150-200 units (45 kelurahan)
- **Optimal density**: 1 unit per 500-750 residents

## **Technical Support**

### **My GPS isn't working. What should I do?**
1. **Check power**: Ensure LDO regulator is properly connected
2. **Verify location**: Move to open area with clear sky view
3. **Check antenna**: Ensure GPS antenna is properly oriented
4. **Monitor serial**: Look for "GPS FIXED" message (takes 5-8 minutes cold start)

### **Why am I getting duplicate SMS alerts?**
This is caused by network latency triggering our guaranteed delivery mechanism. Solutions:

1. **Wait 5 minutes**: System auto-resolves duplicates
2. **Check signal strength**: Ensure > -95 dBm
3. **Update firmware**: Latest version reduces duplication to 0.5%

### **How do I calibrate the MQ-135 gas sensor?**
```cpp
// Calibration procedure
1. Place sensor in clean air for 10 minutes
2. Run calibration command via serial:
   AT+CALIBRATE=MQ135
3. Wait 120 seconds for pre-heat
4. System stores baseline (R0) in EEPROM
```

### **What do the LED colors mean on the device?**
- **Green**: Normal operation (0-50% capacity)
- **Yellow**: Medium priority (51-90% capacity)
- **Red**: Critical (90%+ capacity or high gas)
- **Blinking red**: System error or maintenance required

## **Data & Privacy**

### **Where is my data stored?**
- **Primary**: Blynk IoT Cloud (EU servers)
- **Backup**: Local SD card on device
- **Export**: CSV format available via dashboard
- **Retention**: 365 days in cloud, indefinite locally

### **Is location data private?**
Yes, with these protections:
1. **Anonymization**: Device IDs not linked to specific addresses
2. **Encryption**: AES-256 for all transmissions
3. **Access control**: Role-based dashboard permissions
4. **GDPR compliance**: Data deletion on request

### **Can I export my data for analysis?**
Yes, multiple formats:
1. **Dashboard export**: CSV, JSON, Excel
2. **API access**: REST API for integration
3. **Real-time stream**: MQTT topics for live data
4. **Monthly reports**: Automated PDF generation

## **Future Development**

### **What features are planned for v2.0?**
1. **Solar power integration**: Eliminate battery changes
2. **AI waste classification**: Camera-based material identification
3. **Predictive analytics**: Machine learning for fill patterns
4. **Multi-language support**: Javanese and local dialect interfaces

### **Will BINSAI work with other smart city systems?**
Yes, planned integrations:
- **Traffic management**: Coordinate collection routes
- **Public health**: Correlation with disease patterns
- **Tourism management**: Adjust for visitor density
- **Educational programs**: Data for environmental curriculum

### **How can I contribute to development?**
1. **Code contributions**: Fork GitHub repository
2. **Testing**: Join beta testing program
3. **Documentation**: Improve translations or tutorials
4. **Research**: Collaborate on academic papers

## **Emergency Procedures**

### **What if the system stops transmitting?**
1. **Check power**: Verify power bank charge
2. **Reset device**: Press reset button for 3 seconds
3. **Check SIM card**: Ensure active with credit

### **How do I report a safety issue?**
Immediately contact:
- **Environmental hazard**: DLHK Yogyakarta hotline

### **What maintenance is required?**
- **Daily**: Visual inspection, dashboard check
- **Weekly**: Sensor cleaning, battery check
- **Monthly**: Full calibration, firmware updates
- **Quarterly**: Hardware inspection, performance review

---

## **Contact Information**

**Technical Support**: `research.binsai@protonmail.com`
**Research Inquiries**: `research.binsai@protonmail.com`

**Office Hours**: Monday-Friday, 08:00-17:00 WIB  
**Lab Location**: MAS Assalafiyyah Mlangi Sleman, Yogyakarta

---

*FAQ Version: 2.0 | Last Updated: 2026-01-15 | Based on ISPO 2026 Research Findings*

---
