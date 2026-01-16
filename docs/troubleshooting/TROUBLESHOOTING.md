# BINSAI Troubleshooting Guide

## **Emergency Procedures**

### **System Completely Unresponsive**
**Symptoms**: No LED indicators, no serial output, not detected by computer

```bash
Emergency Checklist:
1. Power Verification:
   - Multimeter check: USB port should show 5V ± 0.25V
   - Current draw: Normal = 120-180mA, Abnormal = <50mA or >500mA
   
2. Hard Reset Procedure:
   a) Disconnect all power sources
   b) Remove and reinsert ESP32 module
   c) Press BOOT button while connecting USB
   d) Release BOOT after 2 seconds
   
3. Minimum Viable Test:
   // Upload blink test sketch
   void setup() { pinMode(2, OUTPUT); }
   void loop() {
     digitalWrite(2, HIGH); delay(500);
     digitalWrite(2, LOW); delay(500);
   }
```

**Expected Outcome**: Blue LED on GPIO2 should blink at 1Hz  
**If Failed**: Hardware defect suspected → Replace ESP32 module

---

### **Critical Alert Not Sent (Life Safety Issue)**
**Symptoms**: Bin >90% full with high gas, but no SMS notification

```cpp
Emergency Diagnostic Routine:
void emergencyDiagnostic() {
    Serial.println("=== EMERGENCY DIAGNOSTIC ===");
    
    // 1. Check GSM module
    Serial.print("GSM Power: ");
    Serial.println(digitalRead(GSM_PWR_KEY) ? "ON" : "OFF");
    
    // 2. Check network registration
    sendATCommand("AT+CREG?", 2000);
    
    // 3. Check SMS storage
    sendATCommand("AT+CPMS?", 2000);
    
    // 4. Force SMS transmission
    String emergencyMsg = "[EMERGENCY] Manual override - ";
    emergencyMsg += "Bin at " + String(latitude, 6);
    emergencyMsg += "," + String(longitude, 6);
    
    sim800l.sendSMS("+62812XXXXXXX", emergencyMsg);
    
    Serial.println("=== DIAGNOSTIC COMPLETE ===");
}
```

**Immediate Actions**:
1. Call municipal waste collection directly
2. Manually check bin and document condition
3. Initiate hardware replacement if GSM faulty

---

## **Hardware Troubleshooting**

### **GPS Module Issues**

#### **Symptom: "GPS NOT FIXED" Error**
**Diagnostic Steps**:
```bash
1. Power Verification:
   - Expected: 3.3V ± 0.1V at GPS VCC pin
   - Actual: [Measure with multimeter]
   - If < 3.2V: Install LDO regulator (Issue #001)

2. Satellite Signal Check:
   $ cat /dev/ttyUSB0  # Monitor raw NMEA data
   Expected: $GPGGA with valid coordinates
   Actual: [Observe output]

3. Antenna Verification:
   - Location: Outdoor with clear sky view
   - Orientation: Flat surface facing upward
   - Connection: Secure SMA/U.FL connector
```

**Solutions**:
- **Cold start wait**: 5-8 minutes normal for first fix
- **Hot start enable**: Backup battery maintains almanac
- **Antenna upgrade**: Active antenna for urban canyons

#### **Symptom: Inaccurate Coordinates (>10m error)**
**Calibration Procedure**:
```python
# calibration/gps_calibration.py
import serial
import pynmea2

def calibrate_gps(reference_lat, reference_lon):
    readings = []
    with serial.Serial('/dev/ttyUSB0', 9600) as ser:
        for _ in range(100):  # Collect 100 samples
            line = ser.readline().decode('ascii', errors='replace')
            if line.startswith('$GPGGA'):
                msg = pynmea2.parse(line)
                readings.append({
                    'lat': msg.latitude,
                    'lon': msg.longitude,
                    'satellites': msg.num_sats
                })
    
    # Calculate average and offset
    avg_lat = sum(r['lat'] for r in readings) / len(readings)
    avg_lon = sum(r['lon'] for r in readings) / len(readings)
    
    offset_lat = reference_lat - avg_lat
    offset_lon = reference_lon - avg_lon
    
    # Apply offset in firmware
    update_gps_offset(offset_lat, offset_lon)
```

---

### **GSM Module Issues**

#### **Symptom: "No Network" or Registration Failed**
**Diagnostic AT Commands**:
```bash
# Enter AT command mode
AT
# Expected: OK

# Check network registration
AT+CREG?
# Expected: +CREG: 0,1 (registered, home network)

# Check signal strength
AT+CSQ
# Expected: +CSQ: 19,99 (RSSI > 10 acceptable)

# Manual network selection
AT+COPS=1,2,"51010"  # Telkomsel
# Wait 30 seconds, then check registration
```

**Troubleshooting Matrix**:
| CSQ Value | Signal Strength | Action Required |
|-----------|----------------|-----------------|
| 0-9 | Very poor | Move device, external antenna |
| 10-14 | Poor | Check antenna connection |
| 15-19 | Fair | Acceptable for SMS |
| 20-31 | Good | Optimal operation |

#### **Symptom: SMS Not Sent (But Network OK)**
**Step-by-Step Debug**:
```cpp
void debugSMSTransmission() {
    Serial.println("=== SMS DEBUG ===");
    
    // 1. Check SIM card
    sendATCommand("AT+CPIN?", 2000);
    // Expected: +CPIN: READY
    
    // 2. Check SMS mode
    sendATCommand("AT+CMGF=1", 2000);  // Text mode
    // Expected: OK
    
    // 3. Check storage
    sendATCommand("AT+CPMS?", 2000);
    // Expected: +CPMS: "SM",x,y
    
    // 4. Test SMS
    sendATCommand("AT+CMGS=\"+62812XXXXXXX\"", 2000);
    Serial.print("Test message");
    Serial.write(26);  // Ctrl+Z to send
    
    // 5. Wait for response
    delay(10000);
    // Expected: +CMGS: <message_id>
}
```

**Common Solutions**:
- **APN configuration**: `AT+CGSOCKCONT=1,"IP","internet"`
- **SMS center**: `AT+CSCA="+6281100000",145`
- **Memory clear**: `AT+CMGD=1,4` (delete all SMS)

---

### **Ultrasonic Sensor Issues**

#### **Symptom: Constant 0cm or 400cm Reading**
**Diagnostic Procedure**:
```bash
Electrical Test:
1. Voltage Check:
   - VCC: 5.0V ± 0.25V
   - GND: 0V (continuity to ground)
   - TRIG/ECHO: 3.3V logic level

2. Signal Test with Oscilloscope:
   - Trigger pulse: 10μs HIGH expected
   - Echo pulse: Duration proportional to distance
   
3. Acoustic Test:
   - Listen for 40kHz clicking sound
   - Use paper to reflect sound near sensor
```

**Firmware Diagnostic**:
```cpp
void testHCSR04() {
    // Test with known distances
    float testDistances[] = {10.0, 20.0, 30.0, 40.0};
    
    for (float expected : testDistances) {
        // Place object at expected distance
        delay(1000);
        
        float measured = ultrasonic.readDistance();
        float error = abs(measured - expected);
        
        Serial.print("Expected: ");
        Serial.print(expected);
        Serial.print("cm, Measured: ");
        Serial.print(measured);
        Serial.print("cm, Error: ");
        Serial.print(error);
        Serial.println("cm");
        
        if (error > 1.0) {
            Serial.println("FAIL: Calibration required");
            calibrateUltrasonic(expected, measured);
        }
    }
}
```

**Calibration Values** (for 40cm bin height):
```cpp
// In config.h
#define SOUND_SPEED 0.0343  // cm/μs
#define MAX_DISTANCE 40.0   // cm
#define MIN_DISTANCE 2.0    // cm (HC-SR04 limitation)

// Calibration offset (if needed)
#define DISTANCE_OFFSET -0.5  // cm
```

---

### **MQ-135 Gas Sensor Issues**

#### **Symptom: Unstable or Drifting Readings**
**Calibration Protocol**:
```python
# calibration/gas_sensor_calibration.py
import time
import numpy as np

class MQ135Calibrator:
    def __init__(self, pin):
        self.pin = pin
        self.R0 = None
        
    def calibrate_in_clean_air(self, duration=300):
        """Calibrate R0 in clean air (5 minutes)"""
        print("Starting clean air calibration...")
        print("Place sensor in clean outdoor air")
        
        readings = []
        start_time = time.time()
        
        while time.time() - start_time < duration:
            adc_value = read_adc(self.pin)
            readings.append(adc_value)
            time.sleep(1)
            
            # Progress indicator
            elapsed = time.time() - start_time
            print(f"Progress: {elapsed:.0f}/{duration}s", end='\r')
        
        # Calculate R0 (resistance in clean air)
        avg_adc = np.mean(readings[-60:])  # Last minute average
        self.R0 = calculate_R0(avg_adc)
        
        print(f"\nCalibration complete: R0 = {self.R0:.2f}")
        return self.R0
    
    def test_with_ammonia_source(self):
        """Test with known ammonia source (for validation)"""
        print("\nAmmonia response test:")
        print("1. Place sensor near ammonia source")
        print("2. Will measure for 60 seconds")
        
        baseline = self.read_ppm()
        print(f"Baseline: {baseline:.1f} ppm")
        
        time.sleep(60)
        
        exposed = self.read_ppm()
        print(f"Exposed: {exposed:.1f} ppm")
        
        increase = ((exposed - baseline) / baseline) * 100
        print(f"Increase: {increase:.1f}%")
        
        if increase < 50:
            print("WARNING: Sensor may need replacement")
```

**Monthly Maintenance Procedure**:
1. **Day 1-7**: Monitor baseline drift
2. **Day 8**: Clean sensor with compressed air
3. **Day 15**: Recalibrate if drift > 10%
4. **Day 30**: Full calibration and health check

---

## **Software Troubleshooting**

### **ESP32 Firmware Issues**

#### **Symptom: System Crashes or Reboots**
**Crash Log Analysis**:
```bash
# Enable core dumps
CONFIG_ESP32_COREDUMP_ENABLE=y
CONFIG_ESP32_COREDUMP_TO_FLASH=y

# Common crash reasons:
1. Stack overflow: Increase stack size
2. Heap fragmentation: Monitor free heap
3. Watchdog timeout: Check blocking operations
4. Memory corruption: Enable heap poisoning
```

**Debug Firmware**:
```cpp
void setup() {
    Serial.begin(115200);
    
    // Enable detailed logging
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    
    // Monitor system health
    xTaskCreate(systemMonitor, "Monitor", 4096, NULL, 1, NULL);
    
    // Check critical components
    if (!checkHardware()) {
        enterSafeMode();
    }
}

void systemMonitor(void *parameter) {
    while (1) {
        Serial.print("Free Heap: ");
        Serial.print(esp_get_free_heap_size());
        Serial.print(" | Min Free: ");
        Serial.print(esp_get_minimum_free_heap_size());
        Serial.print(" | Task: ");
        Serial.println(uxTaskGetStackHighWaterMark(NULL));
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
```

#### **Symptom: Wi-Fi Connection Drops**
**Connection Resilience**:
```cpp
class WiFiManager {
private:
    int connectionAttempts = 0;
    
public:
    bool connectWithRetry(String ssid, String password) {
        while (connectionAttempts < 5) {
            WiFi.begin(ssid.c_str(), password.c_str());
            
            int waitTime = 0;
            while (WiFi.status() != WL_CONNECTED && waitTime < 10000) {
                delay(500);
                waitTime += 500;
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                connectionAttempts = 0;
                return true;
            }
            
            connectionAttempts++;
            delay(1000 * pow(2, connectionAttempts)); // Exponential backoff
        }
        
        // Fallback to GSM
        switchToGSMMode();
        return false;
    }
};
```

**Network Diagnostics**:
```bash
# Useful AT commands for ESP32
AT+CWMODE=1      # Station mode
AT+CWLAP         # List available networks
AT+CWJAP="SSID","PASS"  # Connect
AT+CWQAP         # Disconnect
AT+CIPSTATUS     # Connection status
```

---

### **Blynk IoT Platform Issues**

#### **Symptom: Dashboard Not Updating**
**Connection Diagnostic**:
```cpp
void checkBlynkConnection() {
    Serial.println("=== BLYNK DIAGNOSTIC ===");
    
    // 1. Check Internet connectivity
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected");
        return;
    }
    
    // 2. Ping Blynk server
    if (Blynk.ping() > 1000) {  // >1 second latency
        Serial.println("High latency to Blynk");
    }
    
    // 3. Check virtual pin connections
    for (int vpin = 0; vpin <= 21; vpin++) {
        if (!Blynk.isVPinConnected(vpin)) {
            Serial.print("VPin ");
            Serial.print(vpin);
            Serial.println(" not connected");
        }
    }
    
    // 4. Test data transmission
    Blynk.virtualWrite(V0, 50);  // Test value
    delay(1000);
    
    Serial.println("=== DIAGNOSTIC COMPLETE ===");
}
```

**Common Solutions**:
1. **Update Blynk token**: Recreate project with new token
2. **Check server region**: Use `BLYNK_DEFAULT_DOMAIN` for correct region
3. **Reduce data rate**: Increase transmission interval from 2s to 5s
4. **Monitor data limits**: Free account = 1000 data points/day

---

## **Data & Analytics Issues**

### **Symptom: Inaccurate Fill Percentage**
**Validation Procedure**:
```python
# validation/volume_validation.py
import pandas as pd
import numpy as np

def validate_volume_measurements(csv_file):
    """Compare sensor readings with manual measurements"""
    df = pd.read_csv(csv_file)
    
    results = {
        'total_samples': len(df),
        'accurate': 0,
        'inaccurate': 0,
        'mean_error': 0,
        'max_error': 0
    }
    
    for index, row in df.iterrows():
        sensor_percent = row['sensor_percentage']
        manual_percent = row['manual_percentage']
        
        error = abs(sensor_percent - manual_percent)
        
        if error <= 2.0:  # 2% tolerance
            results['accurate'] += 1
        else:
            results['inaccurate'] += 1
            print(f"Sample {index}: Sensor={sensor_percent}%, "
                  f"Manual={manual_percent}%, Error={error:.1f}%")
        
        results['mean_error'] += error
        results['max_error'] = max(results['max_error'], error)
    
    results['mean_error'] /= len(df)
    results['accuracy_rate'] = (results['accurate'] / len(df)) * 100
    
    return results
```

**Calibration Adjustment**:
```cpp
// If validation shows consistent bias
float calibratePercentage(float rawPercentage) {
    // Linear correction: y = mx + b
    float calibrated = (1.02 * rawPercentage) - 0.5;
    
    // Clamp to 0-100%
    if (calibrated < 0) calibrated = 0;
    if (calibrated > 100) calibrated = 100;
    
    return calibrated;
}
```

---

## **Power Management Issues**

### **Symptom: Short Battery Life**
**Power Consumption Analysis**:
```cpp
void measurePowerConsumption() {
    // Measure current in different modes
    Serial.println("=== POWER ANALYSIS ===");
    
    // 1. Sleep mode
    esp_sleep_enable_timer_wakeup(60 * 1000000);  // 60 seconds
    Serial.println("Sleep current: ~10μA");
    
    // 2. Active sensing (no transmission)
    Serial.println("Sense-only current: ~80mA");
    
    // 3. Wi-Fi transmission
    Serial.println("Wi-Fi TX current: ~120mA");
    
    // 4. GSM transmission
    Serial.println("GSM TX current: ~200mA (peak)");
    
    Serial.println("=== ANALYSIS COMPLETE ===");
}
```

**Optimization Checklist**:
- [ ] Enable deep sleep between measurements
- [ ] Reduce GPS update frequency (10s → 60s)
- [ ] Batch sensor readings before transmission
- [ ] Disable unused peripherals (Bluetooth, LED)
- [ ] Use efficient data encoding (binary vs JSON)

**Expected Battery Life**:
| Mode | Update Interval | Current | 10,000mAh Duration |
|------|----------------|---------|-------------------|
| Deep Sleep | 5 minutes | 10μA | 11.4 years |
| Normal | 2 seconds | 80mA | 5.2 days |
| Critical | 1 second | 120mA | 2.3 days |
| GSM Active | 10 seconds | 200mA | 2.1 days |

---

## **Preventive Maintenance Schedule**

### **Daily Checks (Operator)**
```bash
1. Visual Inspection:
   - LED status lights
   - Physical damage
   - Clean sensor surfaces

2. Dashboard Verification:
   - All units online
   - No critical alerts
   - Data transmission normal

3. Log Review:
   - Error count < 5/day
   - Battery level > 20%
   - GPS accuracy < 5m
```

### **Weekly Maintenance (Technician)**
```bash
1. Hardware Inspection:
   - Tighten all connections
   - Clean waterproof seals
   - Check antenna positioning

2. Sensor Calibration:
   - Ultrasonic: Distance validation
   - Gas: Baseline verification
   - GPS: Coordinate accuracy

3. Data Backup:
   - Export weekly reports
   - Check cloud synchronization
   - Verify local storage
```

### **Monthly Service (Technical Team)**
```bash
1. Comprehensive Testing:
   - Full system diagnostic
   - Battery capacity test
   - Network performance test

2. Firmware Updates:
   - Check for updates
   - Backup current version
   - Deploy with validation

3. Performance Review:
   - Analyze error logs
   - Calculate system uptime
   - Update maintenance records
```

---

## **Support Escalation**

### **Level 1: Self-Service** (Immediate)
- Check this troubleshooting guide
- Review GitHub Issues for similar problems
- Consult FAQ documentation

### **Level 2: Community Support** (24-hour response)
- Post on GitHub Discussions
- Join Telegram support group
- Email: community-support@binsai.id

### **Level 3: Technical Support** (4-hour response)
- Email: technical-support@binsai.id
- Include: Device ID, error logs, photos
- Response: Step-by-step guidance

### **Level 4: On-Site Support** (24-hour emergency)
- Critical system failures
- Safety hazards
- Municipal service disruption
- Contact: +62-812-XXXX-XXXX

---

## **Quick Reference Guide**

### **Error Codes Reference**
| Code | Meaning | Action |
|------|---------|--------|
| **E001** | GPS timeout | Check antenna, move to open area |
| **E002** | GSM no network | Check SIM card, signal strength |
| **E003** | Sensor fault | Clean sensor, recalibrate |
| **E004** | Memory full | Clear logs, increase storage |
| **E005** | Battery low | Charge immediately |
| **E006** | Network timeout | Check Wi-Fi, switch to GSM |

### **LED Status Indicators**
| Pattern | Color | Meaning |
|---------|-------|---------|
| **Solid Green** | Green | Normal operation |
| **Blinking Green** | Green | Data transmission |
| **Solid Yellow** | Yellow | Medium priority |
| **Blinking Yellow** | Yellow | Calibration mode |
| **Solid Red** | Red | Critical alert |
| **Fast Blinking Red** | Red | System error |
| **Alternating R/G** | Red/Green | Boot/initialization |

### **AT Command Quick Reference**
```bash
# System
AT          # Test communication
AT+GMR      # Get version info
AT+RST      # Reset module

# Network
AT+CSQ      # Signal quality
AT+CREG?    # Network registration
AT+COPS?    # Current operator

# GPS
AT+CGPS=1   # Enable GPS
AT+CGPSINFO # Get position
AT+CGPS=0   # Disable GPS

# SMS
AT+CMGF=1   # Text mode
AT+CMGS     # Send SMS
AT+CMGL     # List SMS
```

---

## **Additional Resources**

1. **Complete Schematics**: `media/`
2. **Firmware Source**: `src/`
3. **Research Papers**: `docs/research/`

---

*Troubleshooting Guide Version: 2.1 | Last Updated: 2026-01-15 | Based on ISPO 2026 Field Testing Experience*
