/**
 * ============================================================================
 * BINSAI Research System - Optimized Production Version
 * Internet of Things (IoT) Based Smart Waste Management System
 * Research Project for ISPO 2026 - Yogyakarta Smart City Initiative
 * ============================================================================
 * 
 * SYSTEM ARCHITECTURE:
 * ESP32 → Multi-sensor array → Blynk Cloud → Priority Notifications
 * 
 * HARDWARE COMPONENTS:
 * - ESP32 DevKit v1 (Dual-core, Wi-Fi/Bluetooth 4.2)
 * - HC-SR04 Ultrasonic Sensor (Volume measurement)
 * - MQ-135 Gas Sensor (Ammonia detection)
 * - NEO-6M GPS Module (Geolocation)
 * - SIM800L GSM Module (SMS notifications)
 * - I2C LCD 16x2 (Local display)
 * - Waterproof IP65 enclosure
 * 
 * SOFTWARE SPECIFICATIONS:
 * - Blynk IoT Platform v1.4.0
 * - TinyGPSPlus v1.1.0
 * - LiquidCrystal_I2C v1.1.4
 * - PlatformIO Framework
 * 
 * RESEARCH PARAMETERS:
 * - Sample interval: 2s (Blynk), 60s (Data logging)
 * - Accuracy: 99.46% (Ultrasonic), R²=0.987 (Gas)
 * - Notification reliability: 100%
 * - Modular deployment time: <10 minutes
 * ============================================================================
 */

// ============================================================================
// SECTION 1: PLATFORM CONFIGURATION & COMPATIBILITY
// ============================================================================

// Force compatibility with ESP32 Arduino Core
#define ARDUINO_ARCH_ESP32
#define ARDUINO_JSON_USE_LONG_LONG 1

// ============================================================================
// SECTION 2: BLYNK IOT PLATFORM CONFIGURATION
// ============================================================================

#define BLYNK_TEMPLATE_ID           "TMPL_YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME         "BINSAI Research System"
#define BLYNK_AUTH_TOKEN            "YOUR_AUTH_TOKEN"
#define BLYNK_PRINT                 Serial
#define BLYNK_USE_SSL               1
#define BLYNK_TIMEOUT_MS            3000

// Virtual Pin Mapping (From Appendix 2 of Research Paper)
#define V0_FILL_PERCENTAGE          0      // Integer 0-100%
#define V1_LED_FULL                 1      // LED: FULL status (>90%)
#define V2_LED_ALMOST_FULL          2      // LED: ALMOST FULL (51-90%)
#define V3_LED_HALF                 3      // LED: HALF (36-50%)
#define V4_LED_EMPTY                4      // LED: EMPTY (0-35%)
#define V5_DISTANCE_RAW             5      // Double: Ultrasonic distance (cm)
#define V6_CAPACITY_STATUS          6      // String: Capacity status text
#define V10_GAS_PPM                 10     // Integer: Gas concentration (0-2000ppm)
#define V11_PRIORITY_LEVEL          11     // Integer: Urgency scale (0-3)
#define V12_WASTE_TYPE              12     // String: Organic/Inorganic classification
#define V13_RECOMMENDATION          13     // String: Operational instructions
#define V20_LATITUDE                20     // Double: GPS latitude
#define V21_LONGITUDE               21     // Double: GPS longitude

// ============================================================================
// SECTION 3: HARDWARE PIN DEFINITIONS (Based on Appendix 1)
// ============================================================================

// Sensor Interfaces
#define PIN_ULTRASONIC_TRIG         GPIO_NUM_5     // HC-SR04 Trigger
#define PIN_ULTRASONIC_ECHO         GPIO_NUM_18    // HC-SR04 Echo
#define PIN_GAS_SENSOR              GPIO_NUM_34    // MQ-135 Analog Output
#define PIN_BUZZER                  GPIO_NUM_25    // Piezo buzzer

// Communication Interfaces
#define PIN_SIM800L_RX              GPIO_NUM_16    // SIM800L UART RX
#define PIN_SIM800L_TX              GPIO_NUM_17    // SIM800L UART TX
#define PIN_SIM800L_PWRKEY          GPIO_NUM_4     // SIM800L Power Key
#define PIN_GPS_RX                  GPIO_NUM_13    // NEO-6M UART RX
#define PIN_GPS_TX                  GPIO_NUM_15    // NEO-6M UART TX

// Display & I2C
#define PIN_I2C_SDA                 GPIO_NUM_21    // LCD I2C Data
#define PIN_I2C_SCL                 GPIO_NUM_22    // LCD I2C Clock
#define LCD_I2C_ADDRESS             0x27          // Default I2C address

// ============================================================================
// SECTION 4: SYSTEM PARAMETERS & THRESHOLDS
// ============================================================================

// Bin Specifications
#define BIN_HEIGHT_CM               40.0f         // Maximum bin height
#define SENSOR_MOUNT_HEIGHT_CM      3.0f          // Ultrasonic sensor mounting offset

// Capacity Thresholds (Based on Section 3.4.1)
#define THRESHOLD_EMPTY_PERCENT     35.0f         // 0-35%: Empty
#define THRESHOLD_HALF_PERCENT      50.0f         // 36-50%: Half
#define THRESHOLD_ALMOST_FULL       90.0f         // 51-90%: Almost Full
#define THRESHOLD_CRITICAL          90.1f         // >90%: Critical (Full)

// Gas Concentration Thresholds (Based on Appendix 5)
#define PPM_CLEAN_AIR_MAX           199.0f        // 0-199 ppm: Clean/Normal
#define PPM_INORGANIC_MAX           449.0f        // 200-449 ppm: Inorganic/Light odor
#define PPM_ORGANIC_L1_MAX          800.0f        // 450-800 ppm: Organic (starting decay)
#define PPM_ORGANIC_L2_MIN          801.0f        // >800 ppm: Critical decomposition

// Gas Sensor Calibration (From Section 4.3.1: PPM = 0.002348 * ADC^2.856)
#define MQ135_COEFFICIENT_A         0.002348f
#define MQ135_COEFFICIENT_B         2.856f
#define MQ135_LOAD_RESISTOR         1.0f          // RL = 1kΩ
#define MQ135_CLEAN_AIR_RATIO       3.6f          // RS/R0 ratio in clean air

// Timing Intervals (Based on Research Methodology)
#define INTERVAL_SENSOR_READ_MS     2000          // 2s interval for Blynk updates
#define INTERVAL_DATA_LOG_MS        60000         // 60s interval for research logs
#define INTERVAL_GPS_CHECK_MS       10000         // 10s GPS validation
#define INTERVAL_DISPLAY_ROTATE_MS  4000          // 4s LCD display rotation
#define INTERVAL_SMS_COOLDOWN_MS    300000        // 5 minutes between SMS batches

// System Constants
#define CALIBRATION_DURATION_MS     60000         // 60s MQ-135 calibration
#define SMS_SEND_TIMEOUT_MS         30000         // 30s SMS transmission timeout
#define GPS_FIX_TIMEOUT_MS          60000         // 60s maximum GPS acquisition
#define WIFI_CONNECT_TIMEOUT_MS     20000         // 20s WiFi connection timeout

// ============================================================================
// SECTION 5: NETWORK & COMMUNICATION CONFIGURATION
// ============================================================================

// WiFi Credentials (Replace with your network)
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Emergency Contact Numbers (International Format Required)
const char* EMERGENCY_NUMBERS[] = {
    "+62_YOUR_NUMBER_PHONE1",     // Primary contact
    "+62_YOUR_NUMBER_PHONE2",     // Secondary contact
    "+62_YOUR_NUMBER_PHONE3"      // Tertiary contact
};
const uint8_t EMERGENCY_NUMBERS_COUNT = 3;

// SMS Message Templates
const char* SMS_TEMPLATE_CRITICAL = 
    "[BINSAI ALERT] Bin #{DEVICE_ID} requires immediate attention\n"
    "Capacity: {CAPACITY}% | NH3: {PPM}ppm\n"
    "Priority: {PRIORITY_LEVEL} - {WASTE_TYPE}\n"
    "Location: https://maps.google.com/?q={LAT},{LNG}\n"
    "Recommendation: {RECOMMENDATION}";

const char* SMS_TEMPLATE_TEST = 
    "[BINSAI TEST] System verification message\n"
    "Device: {DEVICE_ID} | Time: {TIMESTAMP}\n"
    "All systems operational. This is a test message.";

// ============================================================================
// SECTION 6: LIBRARY INCLUSIONS
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPSPlus.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// ============================================================================
// SECTION 7: DATA STRUCTURES & TYPE DEFINITIONS
// ============================================================================

/**
 * Sensor Data Structure
 * Contains all telemetry data from sensor array
 */
typedef struct {
    // Ultrasonic Sensor Data
    float distance_cm;              // Measured distance in centimeters
    float fill_percentage;          // Calculated fill percentage (0-100%)
    
    // Gas Sensor Data
    uint16_t adc_raw;              // Raw ADC value from MQ-135
    float ppm_calculated;           // Calculated PPM value
    float r0_calibrated;            // Calibrated R0 value
    
    // GPS Data
    double latitude;                // Latitude in decimal degrees
    double longitude;               // Longitude in decimal degrees
    uint8_t satellite_count;        // Number of visible satellites
    float hdop;                     // Horizontal dilution of precision
    
    // Derived Classification
    uint8_t capacity_level;         // 0=Empty, 1=Half, 2=Almost Full, 3=Full
    uint8_t waste_classification;   // 0=Clean, 1=Inorganic, 2=Organic L1, 3=Organic L2
    uint8_t priority_level;         // 0-3 scale (0=Normal, 3=Critical)
    
    // Timestamps
    uint32_t timestamp_unix;        // Unix timestamp
    uint32_t timestamp_millis;      // Millisecond timestamp
} SensorData_t;

/**
 * System Configuration Structure
 * Contains all configurable system parameters
 */
typedef struct {
    // Device Identification
    char device_id[32];             // Unique device identifier
    char firmware_version[16];      // Firmware version string
    
    // Network Configuration
    char wifi_ssid[32];             // WiFi SSID
    char wifi_password[32];         // WiFi password
    char blynk_auth_token[34];      // Blynk authentication token
    
    // Calibration Parameters
    float ultrasonic_offset_cm;     // Ultrasonic sensor mounting offset
    float mq135_r0_calibrated;      // Calibrated R0 value for MQ-135
    float mq135_temp_compensation;  // Temperature compensation factor
    float mq135_humidity_compensation; // Humidity compensation factor
    
    // Operational Thresholds
    float critical_capacity_threshold;  // Capacity threshold for critical alerts
    float critical_gas_threshold;       // Gas threshold for critical alerts
    uint32_t sms_cooldown_period;       // Minimum time between SMS batches
    
    // Modular Configuration
    bool is_modular_unit;           // True if device is modular deployment
    uint8_t deployment_zone;        // Deployment zone identifier
    char location_description[64];  // Human-readable location description
} SystemConfig_t;

/**
 * Notification State Structure
 * Manages notification system state
 */
typedef struct {
    bool sms_notification_pending;  // True if SMS notification required
    bool sms_in_progress;           // True if SMS transmission active
    bool sms_last_success;          // Status of last SMS transmission
    
    uint8_t sms_recipient_index;    // Current recipient index
    uint8_t sms_sent_count;         // Number of SMS sent in current batch
    uint8_t sms_failed_count;       // Number of failed SMS attempts
    
    uint32_t last_sms_timestamp;    // Timestamp of last SMS transmission
    uint32_t sms_start_timestamp;   // Timestamp when SMS batch started
    
    char sms_message_buffer[320];   // Buffer for SMS message construction
    char sms_recipient_buffer[20];  // Buffer for recipient number
} NotificationState_t;

// ============================================================================
// SECTION 8: GLOBAL OBJECT INSTANCES
// ============================================================================

// Hardware Interfaces
LiquidCrystal_I2C lcd_display(LCD_I2C_ADDRESS, 16, 2);
TinyGPSPlus gps_parser;
HardwareSerial gps_serial(1);      // UART1 for GPS
HardwareSerial gsm_serial(2);      // UART2 for GSM
Preferences nvs_storage;           // Non-volatile storage

// Data Instances
SensorData_t current_sensor_data = {0};
SystemConfig_t system_config = {0};
NotificationState_t notification_state = {0};

// System State Variables
volatile bool system_initialized = false;
volatile bool wifi_connected = false;
volatile bool blynk_connected = false;
volatile bool gps_valid_fix = false;
volatile bool gsm_module_ready = false;
volatile bool calibration_complete = false;
volatile bool critical_condition_active = false;

// Timing Variables
uint32_t last_sensor_update = 0;
uint32_t last_data_log = 0;
uint32_t last_display_update = 0;
uint32_t last_gps_check = 0;
uint32_t system_start_time = 0;

// Rolling averages for sensor stabilization
float distance_rolling_avg[10] = {0};
float ppm_rolling_avg[10] = {0};
uint8_t rolling_avg_index = 0;

// ============================================================================
// SECTION 9: CORE SYSTEM INITIALIZATION
// ============================================================================

/**
 * Initialize all hardware components with validation
 * @return true if all components initialized successfully
 */
bool initializeHardwareComponents() {
    Serial.println("[INIT] Starting hardware initialization sequence...");
    
    // 1. Initialize Serial Communication
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\n==========================================");
    Serial.println("     BINSAI RESEARCH SYSTEM v2.0");
    Serial.println("     ISPO 2026 - Smart Waste Management");
    Serial.println("==========================================");
    
    // 2. Initialize GPIO Pins
    pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ECHO, INPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_SIM800L_PWRKEY, OUTPUT);
    
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_SIM800L_PWRKEY, LOW);
    
    // 3. Initialize I2C Bus
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    delay(100);
    
    // 4. Initialize LCD Display
    if (!initializeLCDDisplay()) {
        Serial.println("[ERROR] LCD display initialization failed");
        return false;
    }
    
    // 5. Initialize UART for GPS and GSM
    gps_serial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    gsm_serial.begin(9600, SERIAL_8N1, PIN_SIM800L_RX, PIN_SIM800L_TX);
    delay(1000);
    
    // 6. Clear UART buffers
    while (gps_serial.available()) gps_serial.read();
    while (gsm_serial.available()) gsm_serial.read();
    
    // 7. Load configuration from NVS
    if (!loadSystemConfiguration()) {
        Serial.println("[WARNING] Using default configuration");
        initializeDefaultConfiguration();
    }
    
    Serial.println("[INIT] Hardware initialization complete");
    return true;
}

/**
 * Initialize LCD display with auto-address detection
 * @return true if LCD initialized successfully
 */
bool initializeLCDDisplay() {
    Serial.print("[LCD] Scanning I2C bus for display... ");
    
    // Try common LCD addresses
    uint8_t addresses[] = {0x27, 0x3F, 0x26, 0x25};
    bool lcd_found = false;
    uint8_t found_address = 0;
    
    for (uint8_t i = 0; i < sizeof(addresses); i++) {
        Wire.beginTransmission(addresses[i]);
        if (Wire.endTransmission() == 0) {
            found_address = addresses[i];
            lcd_found = true;
            break;
        }
    }
    
    if (!lcd_found) {
        Serial.println("NOT FOUND");
        return false;
    }
    
    Serial.print("FOUND at 0x");
    Serial.println(found_address, HEX);
    
    // Reinitialize LCD with found address
    lcd_display = LiquidCrystal_I2C(found_address, 16, 2);
    lcd_display.init();
    lcd_display.backlight();
    lcd_display.clear();
    
    // Display boot screen
    lcd_display.setCursor(0, 0);
    lcd_display.print("BINSAI v2.0");
    lcd_display.setCursor(0, 1);
    lcd_display.print("Initializing...");
    
    return true;
}

// ============================================================================
// SECTION 10: SYSTEM CONFIGURATION MANAGEMENT
// ============================================================================

/**
 * Load system configuration from non-volatile storage
 * @return true if configuration loaded successfully
 */
bool loadSystemConfiguration() {
    if (!nvs_storage.begin("binsai_cfg", false)) {
        Serial.println("[CONFIG] Failed to open NVS storage");
        return false;
    }
    
    // Load device identification
    String device_id = nvs_storage.getString("device_id", "");
    if (device_id.length() > 0) {
        strncpy(system_config.device_id, device_id.c_str(), 
                sizeof(system_config.device_id) - 1);
    } else {
        // Generate unique device ID based on MAC address
        String mac = WiFi.macAddress();
        snprintf(system_config.device_id, sizeof(system_config.device_id),
                 "BINSAI-%s", mac.substring(9).c_str());
        nvs_storage.putString("device_id", system_config.device_id);
    }
    
    // Load calibration data
    system_config.mq135_r0_calibrated = nvs_storage.getFloat("mq135_r0", 10.0f);
    system_config.ultrasonic_offset_cm = nvs_storage.getFloat("us_offset", 3.0f);
    
    // Load operational parameters
    system_config.critical_capacity_threshold = nvs_storage.getFloat("crit_cap", 90.0f);
    system_config.critical_gas_threshold = nvs_storage.getFloat("crit_gas", 800.0f);
    system_config.sms_cooldown_period = nvs_storage.getUInt("sms_cd", 300000);
    
    // Load network configuration
    String wifi_ssid = nvs_storage.getString("wifi_ssid", "");
    if (wifi_ssid.length() > 0) {
        strncpy(system_config.wifi_ssid, wifi_ssid.c_str(),
                sizeof(system_config.wifi_ssid) - 1);
    }
    
    String wifi_pass = nvs_storage.getString("wifi_pass", "");
    if (wifi_pass.length() > 0) {
        strncpy(system_config.wifi_password, wifi_pass.c_str(),
                sizeof(system_config.wifi_password) - 1);
    }
    
    nvs_storage.end();
    
    Serial.println("[CONFIG] Configuration loaded successfully");
    Serial.printf("[CONFIG] Device ID: %s\n", system_config.device_id);
    Serial.printf("[CONFIG] MQ135 R0: %.2f\n", system_config.mq135_r0_calibrated);
    
    return true;
}

/**
 * Initialize default system configuration
 */
void initializeDefaultConfiguration() {
    // Set firmware version
    strcpy(system_config.firmware_version, "2.0.0");
    
    // Generate device ID from MAC address
    String mac = WiFi.macAddress();
    snprintf(system_config.device_id, sizeof(system_config.device_id),
             "BINSAI-%s", mac.substring(9).c_str());
    
    // Default calibration values (from research paper)
    system_config.mq135_r0_calibrated = 10.0f;
    system_config.ultrasonic_offset_cm = 3.0f;
    
    // Default thresholds (from research paper)
    system_config.critical_capacity_threshold = 90.0f;
    system_config.critical_gas_threshold = 800.0f;
    system_config.sms_cooldown_period = 300000;
    
    // Default network configuration (user must update)
    strcpy(system_config.wifi_ssid, "YOUR_WIFI_SSID");
    strcpy(system_config.wifi_password, "YOUR_WIFI_PASSWORD");
    strcpy(system_config.blynk_auth_token, "YOUR_AUTH_TOKEN");
    
    // Modular deployment defaults
    system_config.is_modular_unit = true;
    system_config.deployment_zone = 0;
    strcpy(system_config.location_description, "Research Laboratory");
}

// ============================================================================
// SECTION 11: SENSOR INTERFACE FUNCTIONS
// ============================================================================

/**
 * Read ultrasonic sensor with error correction
 * @return Distance in centimeters, or -1 on error
 */
float readUltrasonicDistance() {
    // Ensure trigger is low
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    
    // Send 10µs pulse
    digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    
    // Measure echo pulse width with timeout
    long duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, 30000);
    
    // Check for timeout or invalid reading
    if (duration == 0 || duration > 30000) {
        Serial.println("[SENSOR] Ultrasonic sensor timeout");
        return -1.0f;
    }
    
    // Calculate distance (sound speed = 343 m/s = 0.0343 cm/µs)
    // Distance = (time × speed) / 2 (round trip)
    float distance_cm = duration * 0.0343f / 2.0f;
    
    // Apply sensor mounting offset
    distance_cm += system_config.ultrasonic_offset_cm;
    
    // Validate range (HC-SR04 range: 2cm to 400cm)
    if (distance_cm < 2.0f || distance_cm > 400.0f) {
        Serial.printf("[SENSOR] Ultrasonic reading out of range: %.2f cm\n", distance_cm);
        return -1.0f;
    }
    
    // Update rolling average
    distance_rolling_avg[rolling_avg_index] = distance_cm;
    
    return distance_cm;
}

/**
 * Read MQ-135 gas sensor with temperature compensation
 * @return Gas concentration in PPM
 */
float readGasConcentration() {
    // Take multiple samples for averaging
    uint32_t adc_sum = 0;
    const uint8_t sample_count = 10;
    
    for (uint8_t i = 0; i < sample_count; i++) {
        adc_sum += analogRead(PIN_GAS_SENSOR);
        delay(10);
    }
    
    uint16_t adc_average = adc_sum / sample_count;
    current_sensor_data.adc_raw = adc_average;
    
    // Apply power-law regression from research: PPM = 0.002348 * ADC^2.856
    float ppm_value = MQ135_COEFFICIENT_A * pow(adc_average, MQ135_COEFFICIENT_B);
    
    // Apply environmental compensation
    ppm_value *= system_config.mq135_temp_compensation;
    ppm_value *= system_config.mq135_humidity_compensation;
    
    // Clamp to valid range
    if (ppm_value < 0) ppm_value = 0;
    if (ppm_value > 2000) ppm_value = 2000;
    
    // Update rolling average
    ppm_rolling_avg[rolling_avg_index] = ppm_value;
    
    return ppm_value;
}

/**
 * Calculate fill percentage from distance measurement
 * @param distance_cm Measured distance from sensor to waste surface
 * @return Fill percentage (0-100%)
 */
float calculateFillPercentage(float distance_cm) {
    if (distance_cm < 0) {
        return 0.0f;  // Invalid reading
    }
    
    // Calculate effective distance (account for sensor mounting)
    float effective_distance = distance_cm - system_config.ultrasonic_offset_cm;
    
    // Calculate percentage based on bin height
    float fill_percentage = (1.0f - (effective_distance / BIN_HEIGHT_CM)) * 100.0f;
    
    // Clamp to valid range
    if (fill_percentage < 0.0f) fill_percentage = 0.0f;
    if (fill_percentage > 100.0f) fill_percentage = 100.0f;
    
    return fill_percentage;
}

/**
 * Get moving average of sensor readings for stabilization
 * @param buffer Array of recent readings
 * @param size Size of the buffer
 * @return Moving average value
 */
float calculateMovingAverage(float buffer[], uint8_t size) {
    float sum = 0.0f;
    uint8_t valid_count = 0;
    
    for (uint8_t i = 0; i < size; i++) {
        if (buffer[i] > 0) {  // Skip invalid readings
            sum += buffer[i];
            valid_count++;
        }
    }
    
    return (valid_count > 0) ? (sum / valid_count) : 0.0f;
}

// ============================================================================
// SECTION 12: GPS MODULE INTERFACE
// ============================================================================

/**
 * Initialize GPS module with cold start
 * @return true if GPS initialized successfully
 */
bool initializeGPSModule() {
    Serial.println("[GPS] Initializing NEO-6M module...");
    
    // Send cold start command
    const char* cold_start_cmds[] = {
        "$PMTK104*37\r\n",  // Cold start
        "$PMTK313,1*2E\r\n", // Enable SBAS
        "$PMTK301,2*2E\r\n", // Enable DGPS
        "$PMTK386,0*23\r\n", // Set NMEA output to 1Hz
    };
    
    for (uint8_t i = 0; i < 4; i++) {
        gps_serial.print(cold_start_cmds[i]);
        delay(100);
    }
    
    // Wait for GPS data
    uint32_t timeout = millis() + GPS_FIX_TIMEOUT_MS;
    bool gps_data_received = false;
    
    while (millis() < timeout) {
        if (gps_serial.available()) {
            char c = gps_serial.read();
            if (c == '$') {
                gps_data_received = true;
                break;
            }
        }
        delay(10);
    }
    
    if (!gps_data_received) {
        Serial.println("[GPS] No data received from module");
        return false;
    }
    
    Serial.println("[GPS] Module initialized, waiting for fix...");
    return true;
}

/**
 * Update GPS data from serial stream
 */
void updateGPSData() {
    while (gps_serial.available() > 0) {
        char c = gps_serial.read();
        gps_parser.encode(c);
    }
    
    // Check if we have a valid fix
    if (gps_parser.location.isValid() && 
        gps_parser.location.isUpdated() &&
        gps_parser.satellites.isValid()) {
        
        current_sensor_data.latitude = gps_parser.location.lat();
        current_sensor_data.longitude = gps_parser.location.lng();
        current_sensor_data.satellite_count = gps_parser.satellites.value();
        current_sensor_data.hdop = gps_parser.hdop.hdop();
        
        // Consider fix valid if we have at least 3 satellites and HDOP < 5.0
        if (current_sensor_data.satellite_count >= 3 && 
            current_sensor_data.hdop < 5.0) {
            gps_valid_fix = true;
        } else {
            gps_valid_fix = false;
        }
    } else {
        gps_valid_fix = false;
    }
    
    // Update timestamp
    if (gps_parser.time.isValid() && 
        gps_parser.date.isValid()) {
        // Convert to Unix timestamp (simplified)
        current_sensor_data.timestamp_unix = 0;  // Would need proper conversion
    }
}

// ============================================================================
// SECTION 13: GSM MODULE INTERFACE
// ============================================================================

/**
 * Initialize SIM800L GSM module
 * @return true if module initialized successfully
 */
bool initializeGSMModule() {
    Serial.println("[GSM] Initializing SIM800L module...");
    
    // Power sequence for SIM800L
    digitalWrite(PIN_SIM800L_PWRKEY, HIGH);
    delay(1500);
    digitalWrite(PIN_SIM800L_PWRKEY, LOW);
    delay(3000);
    
    // Test AT command
    if (!sendGSMCommand("AT", "OK", 2000)) {
        Serial.println("[GSM] Module not responding");
        return false;
    }
    
    // Disable echo
    if (!sendGSMCommand("ATE0", "OK", 1000)) {
        Serial.println("[GSM] Failed to disable echo");
    }
    
    // Set SMS text mode
    if (!sendGSMCommand("AT+CMGF=1", "OK", 1000)) {
        Serial.println("[GSM] Failed to set SMS mode");
        return false;
    }
    
    // Set SMS storage to SIM
    if (!sendGSMCommand("AT+CPMS=\"SM\",\"SM\",\"SM\"", "OK", 2000)) {
        Serial.println("[GSM] Failed to set SMS storage");
    }
    
    // Check SIM card status
    if (!sendGSMCommand("AT+CPIN?", "READY", 5000)) {
        Serial.println("[GSM] SIM card not ready");
        return false;
    }
    
    // Check network registration
    if (!sendGSMCommand("AT+CREG?", "+CREG: 0,1", 10000)) {
        if (!sendGSMCommand("AT+CREG?", "+CREG: 0,5", 5000)) {
            Serial.println("[GSM] Network registration pending");
        }
    }
    
    // Get signal quality
    String response = sendGSMCommandWithResponse("AT+CSQ", 2000);
    Serial.printf("[GSM] Signal quality: %s\n", response.c_str());
    
    Serial.println("[GSM] Module initialized successfully");
    gsm_module_ready = true;
    return true;
}

/**
 * Send AT command to GSM module
 * @param command AT command string
 * @param expected_response Expected response substring
 * @param timeout_ms Timeout in milliseconds
 * @return true if expected response received
 */
bool sendGSMCommand(const char* command, const char* expected_response, uint32_t timeout_ms) {
    gsm_serial.println(command);
    
    uint32_t start_time = millis();
    String response = "";
    
    while (millis() - start_time < timeout_ms) {
        while (gsm_serial.available()) {
            char c = gsm_serial.read();
            response += c;
            
            if (expected_response && response.indexOf(expected_response) >= 0) {
                return true;
            }
            
            if (response.indexOf("ERROR") >= 0) {
                return false;
            }
        }
        delay(10);
    }
    
    return false;
}

/**
 * Send AT command and return response
 * @param command AT command string
 * @param timeout_ms Timeout in milliseconds
 * @return Response string
 */
String sendGSMCommandWithResponse(const char* command, uint32_t timeout_ms) {
    gsm_serial.println(command);
    
    uint32_t start_time = millis();
    String response = "";
    
    while (millis() - start_time < timeout_ms) {
        while (gsm_serial.available()) {
            char c = gsm_serial.read();
            response += c;
        }
        delay(10);
    }
    
    return response;
}

/**
 * Send SMS message to specified number
 * @param phone_number Recipient phone number (international format)
 * @param message SMS message content
 * @return true if SMS sent successfully
 */
bool sendSMSMessage(const char* phone_number, const char* message) {
    if (!gsm_module_ready) {
        Serial.println("[SMS] GSM module not ready");
        return false;
    }
    
    // Set recipient number
    String command = "AT+CMGS=\"";
    command += phone_number;
    command += "\"";
    
    if (!sendGSMCommand(command.c_str(), ">", 5000)) {
        Serial.println("[SMS] Failed to set recipient");
        return false;
    }
    
    // Send message content
    gsm_serial.print(message);
    delay(100);
    
    // Send Ctrl+Z to indicate end of message
    gsm_serial.write(26);
    
    // Wait for confirmation
    if (sendGSMCommand("", "OK", 10000)) {
        Serial.printf("[SMS] Message sent to %s\n", phone_number);
        return true;
    }
    
    Serial.println("[SMS] Failed to send message");
    return false;
}

// ============================================================================
// SECTION 14: DATA CLASSIFICATION & ANALYSIS
// ============================================================================

/**
 * Classify waste based on gas concentration and fill level
 */
void classifyWasteData() {
    // Determine capacity level
    if (current_sensor_data.fill_percentage <= THRESHOLD_EMPTY_PERCENT) {
        current_sensor_data.capacity_level = 0;  // Empty
    } else if (current_sensor_data.fill_percentage <= THRESHOLD_HALF_PERCENT) {
        current_sensor_data.capacity_level = 1;  // Half
    } else if (current_sensor_data.fill_percentage <= THRESHOLD_ALMOST_FULL) {
        current_sensor_data.capacity_level = 2;  // Almost Full
    } else {
        current_sensor_data.capacity_level = 3;  // Full
    }
    
    // Determine waste classification based on PPM
    if (current_sensor_data.ppm_calculated <= PPM_CLEAN_AIR_MAX) {
        current_sensor_data.waste_classification = 0;  // Clean/Normal
        current_sensor_data.priority_level = 0;         // Normal priority
    } else if (current_sensor_data.ppm_calculated <= PPM_INORGANIC_MAX) {
        current_sensor_data.waste_classification = 1;  // Inorganic
        current_sensor_data.priority_level = 1;         // Medium priority
    } else if (current_sensor_data.ppm_calculated <= PPM_ORGANIC_L1_MAX) {
        current_sensor_data.waste_classification = 2;  // Organic Level 1
        current_sensor_data.priority_level = 2;         // High priority
    } else {
        current_sensor_data.waste_classification = 3;  // Organic Level 2
        current_sensor_data.priority_level = 3;         // Critical priority
    }
    
    // Check for critical condition (both capacity >90% AND gas >800ppm)
    critical_condition_active = 
        (current_sensor_data.fill_percentage > system_config.critical_capacity_threshold) &&
        (current_sensor_data.ppm_calculated > system_config.critical_gas_threshold);
}

// ============================================================================
// SECTION 15: BLYNK IOT PLATFORM INTEGRATION
// ============================================================================

/**
 * Connect to Blynk IoT platform
 * @return true if connection successful
 */
bool connectBlynkPlatform() {
    Serial.println("[BLYNK] Connecting to Blynk IoT platform...");
    
    // Configure Blynk with authentication token
    Blynk.config(system_config.blynk_auth_token);
    
    // Attempt connection with timeout
    uint32_t start_time = millis();
    bool connected = false;
    
    while (millis() - start_time < 10000) {
        if (Blynk.connect()) {
            connected = true;
            break;
        }
        delay(500);
    }
    
    if (connected) {
        blynk_connected = true;
        Serial.println("[BLYNK] Connected successfully");
        
        // Send system startup event
        Blynk.logEvent("system_start", 
            String("BINSAI Device ") + system_config.device_id + " is now online");
        
        return true;
    } else {
        Serial.println("[BLYNK] Connection failed");
        blynk_connected = false;
        return false;
    }
}

/**
 * Update all Blynk virtual pins with current data
 */
void updateBlynkVirtualPins() {
    if (!blynk_connected) {
        return;
    }
    
    try {
        // Capacity Data (V0-V6)
        Blynk.virtualWrite(V0_FILL_PERCENTAGE, (int)current_sensor_data.fill_percentage);
        Blynk.virtualWrite(V5_DISTANCE_RAW, current_sensor_data.distance_cm);
        
        // LED Status Indicators (V1-V4)
        Blynk.virtualWrite(V1_LED_FULL, (current_sensor_data.capacity_level == 3) ? 255 : 0);
        Blynk.virtualWrite(V2_LED_ALMOST_FULL, (current_sensor_data.capacity_level == 2) ? 255 : 0);
        Blynk.virtualWrite(V3_LED_HALF, (current_sensor_data.capacity_level == 1) ? 255 : 0);
        Blynk.virtualWrite(V4_LED_EMPTY, (current_sensor_data.capacity_level == 0) ? 255 : 0);
        
        // Gas Sensor Data (V10)
        Blynk.virtualWrite(V10_GAS_PPM, (int)current_sensor_data.ppm_calculated);
        
        // Classification Data (V11-V13)
        Blynk.virtualWrite(V11_PRIORITY_LEVEL, current_sensor_data.priority_level);
        
        // Determine waste type string
        const char* waste_type_str;
        switch (current_sensor_data.waste_classification) {
            case 0: waste_type_str = "CLEAN"; break;
            case 1: waste_type_str = "INORGANIC"; break;
            case 2: waste_type_str = "ORGANIC L1"; break;
            case 3: waste_type_str = "ORGANIC L2"; break;
            default: waste_type_str = "UNKNOWN"; break;
        }
        Blynk.virtualWrite(V12_WASTE_TYPE, waste_type_str);
        
        // Determine recommendation
        const char* recommendation_str;
        switch (current_sensor_data.priority_level) {
            case 0: recommendation_str = "Monitor only"; break;
            case 1: recommendation_str = "Schedule routine collection"; break;
            case 2: recommendation_str = "Prepare special bags and compartments"; break;
            case 3: recommendation_str = "URGENT: Collect immediately before decomposition"; break;
            default: recommendation_str = "Check system"; break;
        }
        Blynk.virtualWrite(V13_RECOMMENDATION, recommendation_str);
        
        // GPS Data (V20-V21)
        if (gps_valid_fix) {
            Blynk.virtualWrite(V20_LATITUDE, current_sensor_data.latitude);
            Blynk.virtualWrite(V21_LONGITUDE, current_sensor_data.longitude);
        }
        
        // Update capacity status text (V6)
        const char* capacity_str;
        switch (current_sensor_data.capacity_level) {
            case 0: capacity_str = "EMPTY"; break;
            case 1: capacity_str = "HALF"; break;
            case 2: capacity_str = "ALMOST FULL"; break;
            case 3: capacity_str = "FULL"; break;
            default: capacity_str = "CALIBRATING"; break;
        }
        Blynk.virtualWrite(V6_CAPACITY_STATUS, capacity_str);
        
    } catch (...) {
        Serial.println("[BLYNK] Error updating virtual pins");
    }
}

// ============================================================================
// SECTION 16: NOTIFICATION MANAGEMENT SYSTEM
// ============================================================================

/**
 * Check if notification conditions are met and trigger alerts
 */
void checkNotificationConditions() {
    // Check SMS cooldown period
    uint32_t current_time = millis();
    if (current_time - notification_state.last_sms_timestamp < 
        system_config.sms_cooldown_period) {
        return;  // Still in cooldown period
    }
    
    // Check for critical condition (both thresholds exceeded)
    if (critical_condition_active && gsm_module_ready) {
        triggerCriticalNotification();
    }
    
    // Check for high capacity only (for different alert type)
    else if (current_sensor_data.fill_percentage > system_config.critical_capacity_threshold) {
        triggerCapacityNotification();
    }
}

/**
 * Trigger critical notification (SMS to all emergency contacts)
 */
void triggerCriticalNotification() {
    if (notification_state.sms_in_progress) {
        return;  // Already processing
    }
    
    Serial.println("[NOTIFY] Critical condition detected! Triggering SMS alerts...");
    
    // Prepare SMS message
    snprintf(notification_state.sms_message_buffer, 
             sizeof(notification_state.sms_message_buffer),
             "[BINSAI CRITICAL ALERT] Device: %s\n"
             "Capacity: %.0f%% | Gas: %.0f ppm\n"
             "Priority: %d | Type: %s\n"
             "Location: https://maps.google.com/?q=%.6f,%.6f\n"
             "Action Required: Immediate collection needed",
             system_config.device_id,
             current_sensor_data.fill_percentage,
             current_sensor_data.ppm_calculated,
             current_sensor_data.priority_level,
             getWasteTypeString(current_sensor_data.waste_classification),
             current_sensor_data.latitude,
             current_sensor_data.longitude);
    
    // Set notification state
    notification_state.sms_notification_pending = true;
    notification_state.sms_in_progress = true;
    notification_state.sms_recipient_index = 0;
    notification_state.sms_sent_count = 0;
    notification_state.sms_failed_count = 0;
    notification_state.sms_start_timestamp = millis();
    
    // Display notification on LCD
    displayNotification("CRITICAL ALERT!", "SMS sending...");
    
    // Log event to Blynk
    if (blynk_connected) {
        Blynk.logEvent("critical_alert", 
            String("Critical condition: ") + 
            String(current_sensor_data.fill_percentage, 0) + "% full, " +
            String(current_sensor_data.ppm_calculated, 0) + " ppm");
    }
}

/**
 * Send SMS notifications to all emergency contacts
 */
void processSMSNotifications() {
    if (!notification_state.sms_in_progress) {
        return;
    }
    
    // Check for timeout
    if (millis() - notification_state.sms_start_timestamp > SMS_SEND_TIMEOUT_MS) {
        Serial.println("[SMS] Notification timeout");
        notification_state.sms_in_progress = false;
        notification_state.sms_last_success = false;
        return;
    }
    
    // Send to next recipient
    if (notification_state.sms_recipient_index < EMERGENCY_NUMBERS_COUNT) {
        const char* recipient = EMERGENCY_NUMBERS[notification_state.sms_recipient_index];
        
        // Skip placeholder numbers
        if (strstr(recipient, "YOUR_NUMBER") != NULL) {
            Serial.printf("[SMS] Skipping placeholder: %s\n", recipient);
            notification_state.sms_recipient_index++;
            return;
        }
        
        // Send SMS
        Serial.printf("[SMS] Sending to: %s\n", recipient);
        
        if (sendSMSMessage(recipient, notification_state.sms_message_buffer)) {
            notification_state.sms_sent_count++;
            Serial.println("[SMS] Sent successfully");
        } else {
            notification_state.sms_failed_count++;
            Serial.println("[SMS] Failed to send");
        }
        
        notification_state.sms_recipient_index++;
        
        // Update display
        displayNotification("SMS Progress", 
            String(notification_state.sms_recipient_index) + "/" + 
            String(EMERGENCY_NUMBERS_COUNT));
        
        delay(2000);  // Delay between SMS sends
        
    } else {
        // All recipients processed
        notification_state.sms_in_progress = false;
        notification_state.last_sms_timestamp = millis();
        notification_state.sms_last_success = 
            (notification_state.sms_sent_count > 0);
        
        // Final status
        Serial.printf("[SMS] Batch complete. Sent: %d, Failed: %d\n",
                     notification_state.sms_sent_count,
                     notification_state.sms_failed_count);
        
        if (notification_state.sms_sent_count > 0) {
            displayNotification("SMS Complete", 
                String(notification_state.sms_sent_count) + " sent");
            beepPattern(3);  // Success beep
        } else {
            displayNotification("SMS Failed", 
                "Check GSM module");
            beepPattern(1);  // Error beep
        }
    }
}

/**
 * Get waste type as string
 * @param classification Waste classification code
 * @return String description
 */
const char* getWasteTypeString(uint8_t classification) {
    switch (classification) {
        case 0: return "CLEAN";
        case 1: return "INORGANIC";
        case 2: return "ORGANIC L1";
        case 3: return "ORGANIC L2";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// SECTION 17: DISPLAY MANAGEMENT
// ============================================================================

/**
 * Display notification on LCD
 * @param title Notification title (line 1)
 * @param message Notification message (line 2)
 */
void displayNotification(const char* title, const char* message) {
    if (!lcd_display || !system_initialized) {
        return;
    }
    
    lcd_display.clear();
    lcd_display.setCursor(0, 0);
    lcd_display.print(title);
    
    lcd_display.setCursor(0, 1);
    
    // Handle long messages with scrolling
    if (strlen(message) <= 16) {
        lcd_display.print(message);
    } else {
        // Simple truncation for now
        char truncated[17];
        strncpy(truncated, message, 16);
        truncated[16] = '\0';
        lcd_display.print(truncated);
    }
}

/**
 * Rotate through display screens
 */
void rotateDisplayScreens() {
    static uint8_t screen_index = 0;
    static uint32_t last_rotate = 0;
    
    if (millis() - last_rotate < INTERVAL_DISPLAY_ROTATE_MS) {
        return;
    }
    
    last_rotate = millis();
    
    if (!lcd_display || notification_state.sms_in_progress) {
        return;
    }
    
    lcd_display.clear();
    
    switch (screen_index % 4) {
        case 0:  // Capacity screen
            lcd_display.setCursor(0, 0);
            lcd_display.print("Capacity:");
            lcd_display.setCursor(0, 1);
            lcd_display.print(current_sensor_data.fill_percentage, 0);
            lcd_display.print("% ");
            lcd_display.print(getCapacityLevelString(current_sensor_data.capacity_level));
            break;
            
        case 1:  // Gas level screen
            lcd_display.setCursor(0, 0);
            lcd_display.print("Gas Level:");
            lcd_display.setCursor(0, 1);
            lcd_display.print(current_sensor_data.ppm_calculated, 0);
            lcd_display.print(" ppm ");
            lcd_display.print(getWasteTypeString(current_sensor_data.waste_classification));
            break;
            
        case 2:  // System status screen
            lcd_display.setCursor(0, 0);
            lcd_display.print("System Status");
            lcd_display.setCursor(0, 1);
            lcd_display.print("GPS:");
            lcd_display.print(gps_valid_fix ? "OK" : "NO");
            lcd_display.print(" SMS:");
            lcd_display.print(gsm_module_ready ? "ON" : "OFF");
            break;
            
        case 3:  // Device info screen
            lcd_display.setCursor(0, 0);
            lcd_display.print("Device:");
            lcd_display.setCursor(0, 1);
            lcd_display.print(system_config.device_id);
            break;
    }
    
    screen_index++;
}

/**
 * Get capacity level as string
 * @param level Capacity level code
 * @return String description
 */
const char* getCapacityLevelString(uint8_t level) {
    switch (level) {
        case 0: return "EMPTY";
        case 1: return "HALF";
        case 2: return "ALMOST";
        case 3: return "FULL";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// SECTION 18: AUDIO FEEDBACK
// ============================================================================

/**
 * Play beep pattern for feedback
 * @param pattern Pattern code (1=error, 2=warning, 3=success)
 */
void beepPattern(uint8_t pattern) {
    switch (pattern) {
        case 1:  // Error pattern
            for (int i = 0; i < 3; i++) {
                digitalWrite(PIN_BUZZER, HIGH);
                delay(100);
                digitalWrite(PIN_BUZZER, LOW);
                delay(100);
            }
            break;
            
        case 2:  // Warning pattern
            for (int i = 0; i < 2; i++) {
                digitalWrite(PIN_BUZZER, HIGH);
                delay(200);
                digitalWrite(PIN_BUZZER, LOW);
                delay(200);
            }
            break;
            
        case 3:  // Success pattern
            digitalWrite(PIN_BUZZER, HIGH);
            delay(50);
            digitalWrite(PIN_BUZZER, LOW);
            delay(50);
            digitalWrite(PIN_BUZZER, HIGH);
            delay(50);
            digitalWrite(PIN_BUZZER, LOW);
            delay(50);
            digitalWrite(PIN_BUZZER, HIGH);
            delay(150);
            digitalWrite(PIN_BUZZER, LOW);
            break;
    }
}

// ============================================================================
// SECTION 19: MAIN SETUP FUNCTION
// ============================================================================

void setup() {
    // Record system start time
    system_start_time = millis();
    
    // Initialize hardware components
    if (!initializeHardwareComponents()) {
        Serial.println("[ERROR] Hardware initialization failed. System halted.");
        while (1) {
            // Blink LED to indicate error
            digitalWrite(PIN_BUZZER, HIGH);
            delay(100);
            digitalWrite(PIN_BUZZER, LOW);
            delay(900);
        }
    }
    
    // Display startup sequence
    displayNotification("BINSAI v2.0", "Initializing...");
    
    // Initialize GSM module
    if (!initializeGSMModule()) {
        Serial.println("[WARNING] GSM module initialization failed");
        displayNotification("GSM Module", "Initialization failed");
        delay(2000);
    }
    
    // Initialize GPS module
    if (!initializeGPSModule()) {
        Serial.println("[WARNING] GPS module initialization failed");
        displayNotification("GPS Module", "Initialization failed");
        delay(2000);
    }
    
    // Connect to WiFi
    Serial.println("[NETWORK] Connecting to WiFi...");
    displayNotification("WiFi", "Connecting...");
    
    WiFi.begin(system_config.wifi_ssid, system_config.wifi_password);
    
    uint32_t wifi_timeout = millis() + WIFI_CONNECT_TIMEOUT_MS;
    while (WiFi.status() != WL_CONNECTED && millis() < wifi_timeout) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected = true;
        Serial.printf("\n[NETWORK] WiFi connected. IP: %s\n", 
                     WiFi.localIP().toString().c_str());
        displayNotification("WiFi Connected", WiFi.localIP().toString().c_str());
        beepPattern(3);
    } else {
        Serial.println("\n[NETWORK] WiFi connection failed");
        displayNotification("WiFi Failed", "Offline mode");
        beepPattern(1);
    }
    
    // Connect to Blynk
    if (wifi_connected) {
        connectBlynkPlatform();
    }
    
    // System ready
    system_initialized = true;
    Serial.println("\n[SYSTEM] Initialization complete. Entering main loop.");
    displayNotification("System Ready", "BINSAI Active");
    
    // Success beep sequence
    beepPattern(3);
    delay(1000);
}

// ============================================================================
// SECTION 20: MAIN LOOP FUNCTION
// ============================================================================

void loop() {
    // Current timestamp for timing control
    uint32_t current_time = millis();
    
    // 1. Handle Blynk connection and events
    if (blynk_connected) {
        Blynk.run();
    } else if (wifi_connected && (current_time % 30000 < 100)) {
        // Attempt reconnection every 30 seconds
        connectBlynkPlatform();
    }
    
    // 2. Read sensors at 2-second interval (for Blynk updates)
    if (current_time - last_sensor_update >= INTERVAL_SENSOR_READ_MS) {
        last_sensor_update = current_time;
        
        // Read ultrasonic sensor
        float distance = readUltrasonicDistance();
        if (distance > 0) {
            current_sensor_data.distance_cm = 
                calculateMovingAverage(distance_rolling_avg, 10);
            current_sensor_data.fill_percentage = 
                calculateFillPercentage(current_sensor_data.distance_cm);
        }
        
        // Read gas sensor
        float ppm = readGasConcentration();
        if (ppm >= 0) {
            current_sensor_data.ppm_calculated = 
                calculateMovingAverage(ppm_rolling_avg, 10);
        }
        
        // Update GPS data
        updateGPSData();
        
        // Classify waste data
        classifyWasteData();
        
        // Update rolling index
        rolling_avg_index = (rolling_avg_index + 1) % 10;
        
        // Update Blynk virtual pins
        updateBlynkVirtualPins();
        
        // Debug output
        Serial.printf("[DATA] Dist: %.1fcm, Fill: %.1f%%, PPM: %.1f, GPS: %s\n",
                     current_sensor_data.distance_cm,
                     current_sensor_data.fill_percentage,
                     current_sensor_data.ppm_calculated,
                     gps_valid_fix ? "OK" : "NO");
    }
    
    // 3. Log data at 60-second interval (for research purposes)
    if (current_time - last_data_log >= INTERVAL_DATA_LOG_MS) {
        last_data_log = current_time;
        logResearchData();
    }
    
    // 4. Check GPS status periodically
    if (current_time - last_gps_check >= INTERVAL_GPS_CHECK_MS) {
        last_gps_check = current_time;
        
        if (!gps_valid_fix) {
            Serial.println("[GPS] No valid fix. Searching for satellites...");
            displayNotification("GPS Status", "Searching...");
        }
    }
    
    // 5. Check notification conditions
    checkNotificationConditions();
    
    // 6. Process pending SMS notifications
    if (notification_state.sms_in_progress) {
        processSMSNotifications();
    }
    
    // 7. Update display
    rotateDisplayScreens();
    
    // 8. Small delay to prevent watchdog timer issues
    delay(10);
}

// ============================================================================
// SECTION 21: RESEARCH DATA LOGGING
// ============================================================================

/**
 * Log research data for analysis
 */
void logResearchData() {
    // This would typically log to SD card or cloud service
    // For now, we'll output to Serial in CSV format
    
    Serial.print("[RESEARCH] ");
    Serial.print(millis()); Serial.print(",");
    Serial.print(current_sensor_data.distance_cm, 2); Serial.print(",");
    Serial.print(current_sensor_data.fill_percentage, 2); Serial.print(",");
    Serial.print(current_sensor_data.ppm_calculated, 2); Serial.print(",");
    Serial.print(current_sensor_data.adc_raw); Serial.print(",");
    Serial.print(gps_valid_fix ? 1 : 0); Serial.print(",");
    Serial.print(current_sensor_data.latitude, 6); Serial.print(",");
    Serial.print(current_sensor_data.longitude, 6); Serial.print(",");
    Serial.print(current_sensor_data.satellite_count); Serial.print(",");
    Serial.print(current_sensor_data.capacity_level); Serial.print(",");
    Serial.print(current_sensor_data.waste_classification); Serial.print(",");
    Serial.println(current_sensor_data.priority_level);
    
    // Update Blynk with log event
    if (blynk_connected) {
        Blynk.virtualWrite(V13_RECOMMENDATION, 
            String("Last log: ") + String(millis() / 1000) + "s");
    }
}

// ============================================================================
// SECTION 22: BLYNK EVENT HANDLERS
// ============================================================================

/**
 * Blynk virtual write handler (if needed)
 */
BLYNK_WRITE(V0_FILL_PERCENTAGE) {
    // Handle writes to virtual pin V0 (if required)
}

/**
 * Blynk connection status handler
 */
BLYNK_CONNECTED() {
    blynk_connected = true;
    Serial.println("[BLYNK] Connected to server");
    
    // Sync all virtual pins on connection
    Blynk.syncAll();
}

/**
 * Blynk disconnection handler
 */
BLYNK_DISCONNECTED() {
    blynk_connected = false;
    Serial.println("[BLYNK] Disconnected from server");
}

// ============================================================================
// END OF BINSAI RESEARCH SYSTEM CODE
// ============================================================================
