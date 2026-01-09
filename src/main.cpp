// BLYNK CONFIGURATION
#define BLYNK_TEMPLATE_ID "TMPL6RvIxrbhK"
#define BLYNK_TEMPLATE_NAME "BINSAI"
#define BLYNK_AUTH_TOKEN "cbbsHBuQ4EfVQkyoryQbnBJBwuY3bzy9" // GANTI DENGAN TOKEN ANDA

// LIBRARIES
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32_SSL.h>  // Blynk dengan SSL
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <Preferences.h>           // Pengganti EEPROM yang aman
// Include file secrets (aman)
#include "secrets.h"

// ============ SECRETS MANAGEMENT ============
// File terpisah "secrets.h" untuk keamanan (simpan di .gitignore)
// #include "secrets.h"  // Uncomment setelah membuat file

// ============ PIN DEFINITION (ESP32) ============
// HC-SR04 Ultrasonic
#define TRIG_PIN 5     // GPIO5 - Output
#define ECHO_PIN 18    // GPIO18 - Input (5V tolerant dengan voltage divider)

// MQ-135 Gas Sensor
#define MQ135_PIN 34   // GPIO34 - Input only (ADC1_CH6)

// SIM800L GSM Module
#define SIM800_RX 16   // ESP32 RX2 → SIM800L TX
#define SIM800_TX 17   // ESP32 TX2 → SIM800L RX
#define SIM800_PWRKEY 27  // Power control SIM800L
#define SIM800_STATUS 26  // Status check (optional)

// GPS NEO-6M
#define GPS_RX 13      // ESP32 RX1 → GPS TX (AMAN untuk boot)
#define GPS_TX 15      // ESP32 TX1 → GPS RX (AMAN untuk boot)

// LCD I2C 16x2 (Address 0x27)
#define I2C_SDA 21     // GPIO21
#define I2C_SCL 22     // GPIO22

// LED Indicator
#define LED_PIN 2      // GPIO2 - LED onboard (hati-hati saat boot)

// Button for Emergency/Reset
#define BUTTON_PIN 0   // GPIO0 - Boot button (gunakan dengan PULLUP)

// ============ WIRING DIAGRAM (POWER MANAGEMENT) ============
/*
Battery 9V, dengan modul XL4015 step-down with heat sink
┌─────────────────────────────────────────────┐
│ PIN OUT (+) → ESP32 (5V) & SIM800L (5VIN)   │
│ PIN OUT (-) → GND ESP32 & GND POWER SIM800L │
└─────────────────────────────────────────────┘

SENSOR WIRING:
1. HC-SR04:
   VCC  → 5V ESP32
   Trig → GPIO5
   Echo → GPIO18 (dengan voltage divider 1kΩ+2kΩ untuk 5V→3.3V)
   GND  → GND

2. MQ-135:
   VCC  → 3.3V ESP32 (atau 5V dengan voltage divider)
   AOUT → GPIO34 (dengan voltage divider jika output 5V)
   DOUT → Not used
   GND  → GND

3. LCD I2C:
   VCC → 5V ESP32
   GND → GND
   SDA → GPIO21
   SCL → GPIO22

4. GPS NEO-6M:
   VCC → 3.3V ESP32 (atau 5V jika modul mendukung)
   GND → GND
   TX  → GPIO13 (RX1 ESP32)
   RX  → GPIO15 (TX1 ESP32)

5. SIM800L:
   5VIN     → 4V dari regulator step-down
   GND     → Common ground
   TX      → GPIO16 (RX2 ESP32)
   RX      → GPIO17 (TX2 ESP32)
   PWRKEY  → GPIO27
   STATUS  → GPIO26 (optional)
*/

// ============ PREFERENCES (Secure Storage) ============
Preferences prefs;
#define PREF_NAMESPACE "binsai_config"

// Konfigurasi dari secrets.h
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;
String phoneNumbers[] = {PHONE_NUMBER_1, PHONE_NUMBER_2};
const int phoneCount = 2;
const float TINGGI_BAK = BIN_HEIGHT_CM;
const float R0 = MQ135_RO_CALIBRATED;
const int UPDATE_INTERVAL = 2000;      // 2 detik
const int DISPLAY_ROTATE_INTERVAL = 3000; // 3 detik untuk rotate display

// MQ-135 Calibration
#define RL_VALUE 10.0                  // Load resistor (kΩ)
#define RO_CLEAN_AIR_FACTOR 9.83       // RS/R0 in clean air
#define CALIBRATION_SAMPLE_TIMES 50    // Number of samples for calibration
#define CALIBRATION_SAMPLE_INTERVAL 500 // ms between samples

// Gas Concentration Thresholds (ppm) - HARUS DIKALIBRASI EMPIRIS!
#define PPM_NORMAL_MAX 200             // Udara bersih/bau minimal
#define PPM_ANORGANIK_MAX 450          // Sampah kering/anorganik
#define PPM_ORGANIK_L1_MAX 800         // Organik mulai membusuk
// >800 = Organik busuk tingkat tinggi

// SMS Configuration
const unsigned long SMS_COOLDOWN = 300000;      // 5 menit
const unsigned long SMS_RESET_THRESHOLD = 80;   // Reset SMS flag jika <80%

// ============ GLOBAL VARIABLES ============
// Sensor Data
float distance = 0, fillPercentage = 0, ppmValue = 0;
float calibratedRo = RO_CLEAN_AIR_FACTOR;
String capacityStatus = "CALIBRATING";
String wasteType = "UNKNOWN", recommendation = "WAITING";
int priorityLevel = 0;

// GPS Data
float gpsLat = 0, gpsLng = 0;
bool gpsValid = false;
int satellites = 0;
unsigned long lastGPSFix = 0;

// System Status
bool sim800Ready = false;
bool blynkConnected = false;
bool mq135Calibrated = false;
bool systemInitialized = false;
unsigned long lastUpdateTime = 0;
unsigned long lastSMSTime = 0;

// SMS State Machine
enum SMSState { SMS_IDLE, SMS_PREPARE, SMS_SENDING, SMS_WAIT_CONFIRM };
SMSState smsState = SMS_IDLE;
unsigned long smsStateTime = 0;
String smsMessage = "";
int smsRecipientIndex = 0;
bool smsSentFlags[4] = {false, false, false, false}; // Untuk 4 priority levels

// Display
LiquidCrystal_I2C lcd(0x27, 16, 2);
int displayMode = 0;

// Hardware Serial
HardwareSerial sim800(1);    // UART2 for SIM800L
HardwareSerial gpsSerial(2); // UART1 for GPS
TinyGPSPlus gps;

// Blynk Timer
BlynkTimer timer;

// ============ VIRTUAL PIN MAPPING (BLYNK) ============
/*
Datastream Configuration in Blynk Cloud:
┌─────────────────────────────────────────────────────────────┐
│ V0  │ Fill Percentage │ INTEGER │ 0-100  │ %               │
│ V1  │ LED Red         │ INTEGER │ 0/255  │ (Full)          │
│ V2  │ LED Orange      │ INTEGER │ 0/255  │ (Almost Full)   │
│ V3  │ LED Yellow      │ INTEGER │ 0/255  │ (Half)          │
│ V4  │ LED Green       │ INTEGER │ 0/255  │ (Empty)         │
│ V5  │ Distance        │ DOUBLE  │ 0-400  │ cm              │
│ V6  │ Capacity Status │ STRING  │ -      │ -               │
│ V10 │ PPM Value       │ INTEGER │ 0-2000 │ ppm             │
│ V11 │ Priority Level  │ INTEGER │ 0-3    │ -               │
│ V12 │ Waste Type      │ STRING  │ -      │ -               │
│ V13 │ Recommendation  │ STRING  │ -      │ -               │
│ V20 │ Latitude        │ DOUBLE  │ -      │ GPS             │
│ V21 │ Longitude       │ DOUBLE  │ -      │ GPS             │
└─────────────────────────────────────────────────────────────┘

Widget Recommendations:
1. SuperChart: V0 (Fill Percentage) & V10 (PPM Value)
2. LED Widgets: V1 (Red), V2 (Orange), V3 (Yellow), V4 (Green)
3. Labeled Value: V5 (Distance in cm)
4. Labeled Value: V6 (Capacity Status) with Color Coding
5. Gauge: V10 (PPM Value) 0-2000
6. Value Display: V11 (Priority Level 0-3)
7. Label: V12 (Waste Type)
8. Label: V13 (Recommendation)
9. Map: V20 & V21 (GPS Location)
*/

// =============================================
// SETUP FUNCTION
// =============================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=================================");
  Serial.println("BINSAI - SMART WASTE SYSTEM");
  Serial.println("For ISPO Research Level");
  Serial.println("=================================");
  
  // Initialize all components with error handling
  initPins();
  initLCD();
  
  // Load secure credentials
  loadCredentials();
  
  // Initialize communication modules
  initSIM800L();
  initGPS();
  
  // Sensor calibration (critical for research)
  calibrateMQ135();
  
  // Network connections
  connectWiFi();
  connectBlynk();
  
  // Setup timers
  timer.setInterval(2000L, updateBlynkData);      // Update Blynk every 2s
  timer.setInterval(10000L, checkSystemHealth);   // Health check every 10s
  timer.setInterval(30000L, attemptReconnections);// Reconnection every 30s
  
  systemInitialized = true;
  Serial.println("\n=== SYSTEM INITIALIZATION COMPLETE ===");
  Serial.println("All systems are GO for research deployment");
  
  lcd.clear();
  lcd.print("BINSAI");
  lcd.setCursor(0, 1);
  lcd.print("Research Ready");
  delay(2000);
}

// =============================================
// PIN INITIALIZATION
// =============================================
void initPins() {
  Serial.println("Initializing GPIO pins...");
  
  // HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  
  // SIM800L Control
  pinMode(SIM800_PWRKEY, OUTPUT);
  pinMode(SIM800_STATUS, INPUT_PULLUP);
  digitalWrite(SIM800_PWRKEY, LOW);
  
  // LED Indicator
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Button (with internal pullup)
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("GPIO initialization complete");
}

// =============================================
// LCD INITIALIZATION
// =============================================
void initLCD() {
  Serial.println("Initializing LCD...");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);
  
  // Try multiple I2C addresses
  byte addresses[] = {0x27, 0x3F};
  bool lcdFound = false;
  
  for (byte addr : addresses) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      lcd = LiquidCrystal_I2C(addr, 16, 2);
      lcd.init();
      lcd.backlight();
      lcdFound = true;
      
      Serial.print("LCD found at 0x");
      Serial.println(addr, HEX);
      
      lcd.clear();
      lcd.print("BINSAI");
      lcd.setCursor(0, 1);
      lcd.print("Initializing...");
      break;
    }
  }
  
  if (!lcdFound) {
    Serial.println("ERROR: LCD not found! Check I2C connections");
    // Continue without LCD for research robustness
  }
}

// =============================================
// SECURE CREDENTIALS MANAGEMENT
// =============================================
void loadCredentials() {
  Serial.println("Loading secure credentials...");
  
  prefs.begin(PREF_NAMESPACE, true); // Read-only mode
  
  // Load WiFi credentials if exist
  String savedSSID = prefs.getString("wifi_ssid", "");
  String savedPass = prefs.getString("wifi_pass", "");
  
  if (savedSSID.length() > 0 && savedPass.length() > 0) {
    savedSSID.toCharArray(ssid, sizeof(ssid));
    savedPass.toCharArray(pass, sizeof(pass));
    Serial.println("Loaded WiFi credentials from secure storage");
  }
  
  // Load phone numbers
  for (int i = 0; i < phoneCount; i++) {
    String key = "phone_" + String(i);
    String savedPhone = prefs.getString(key.c_str(), "");
    if (savedPhone.length() > 0) {
      phoneNumbers[i] = savedPhone;
    }
  }
  
  prefs.end();
}

void saveCredentials() {
  prefs.begin(PREF_NAMESPACE, false); // Read-write mode
  
  prefs.putString("wifi_ssid", ssid);
  prefs.putString("wifi_pass", pass);
  
  for (int i = 0; i < phoneCount; i++) {
    String key = "phone_" + String(i);
    prefs.putString(key.c_str(), phoneNumbers[i]);
  }
  
  prefs.end();
  Serial.println("Credentials saved to secure storage");
}

// =============================================
// SIM800L POWER MANAGEMENT
// =============================================
void powerOnSIM800L() {
  Serial.println("Powering SIM800L with proper sequence...");
  
  // Ensure PWRKEY is low first
  digitalWrite(SIM800_PWRKEY, LOW);
  delay(100);
  
  // SIM800L power-on sequence: PWRKEY high for 1-2 seconds
  digitalWrite(SIM800_PWRKEY, HIGH);
  delay(1500); // 1.5 seconds as per datasheet
  digitalWrite(SIM800_PWRKEY, LOW);
  
  // Wait for module to boot (3-5 seconds)
  delay(5000);
}

void initSIM800L() {
  Serial.println("Initializing SIM800L module...");
  lcd.clear();
  lcd.print("GSM Module");
  lcd.setCursor(0, 1);
  lcd.print("Powering ON...");
  
  // Power cycle the module
  powerOnSIM800L();
  
  // Initialize hardware serial for SIM800L
  sim800.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);
  delay(1000);
  
  // Test communication
  sendATCommand("AT", 1000);
  sendATCommand("ATE0", 1000);  // Echo off
  
  // Configure SMS settings
  if (sendATCommand("AT+CMGF=1", 1000)) {  // Text mode
    sendATCommand("AT+CNMI=1,2,0,0,0", 1000);  // New SMS indication
    sim800Ready = true;
    
    Serial.println("SIM800L initialized successfully");
    lcd.clear();
    lcd.print("GSM: Ready");
    lcd.setCursor(0, 1);
    lcd.print("SMS Enabled");
  } else {
    sim800Ready = false;
    Serial.println("ERROR: SIM800L initialization failed");
    lcd.clear();
    lcd.print("GSM: ERROR");
    lcd.setCursor(0, 1);
    lcd.print("Check Power/Sim");
  }
  
  delay(2000);
}

bool sendATCommand(String command, unsigned long timeout) {
  sim800.println(command);
  
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < timeout) {
    while (sim800.available()) {
      char c = sim800.read();
      response += c;
    }
    
    if (response.indexOf("OK") != -1) {
      return true;
    }
    if (response.indexOf("ERROR") != -1) {
      return false;
    }
    
    delay(10);
  }
  
  return false;
}

// =============================================
// GPS INITIALIZATION
// =============================================
void initGPS() {
  Serial.println("Initializing GPS module...");
  
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  delay(1000);
  
  lcd.clear();
  lcd.print("GPS Module");
  lcd.setCursor(0, 1);
  lcd.print("Searching...");
  
  Serial.println("GPS serial communication established");
}

// =============================================
// MQ-135 CALIBRATION (CRITICAL FOR RESEARCH)
// =============================================
void calibrateMQ135() {
  Serial.println("Starting MQ-135 calibration...");
  Serial.println("IMPORTANT: Ensure sensor is in clean air!");
  
  lcd.clear();
  lcd.print("MQ-135 Calib");
  lcd.setCursor(0, 1);
  lcd.print("Clean Air 60s...");
  
  // Warm-up period (minimum 60 seconds for research accuracy)
  for (int i = 60; i > 0; i--) {
    lcd.setCursor(0, 1);
    lcd.print("Warm-up: " + String(i) + "s ");
    delay(1000);
  }
  
  // Take multiple samples for accuracy
  float sensorVoltageSum = 0;
  
  lcd.clear();
  lcd.print("Sampling...");
  lcd.setCursor(0, 1);
  
  for (int i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) {
    int sensorValue = analogRead(MQ135_PIN);
    float sensorVoltage = sensorValue * (3.3 / 4095.0);
    sensorVoltageSum += sensorVoltage;
    
    if (i % 10 == 0) {
      lcd.setCursor(0, 1);
      lcd.print("Sample " + String(i) + "/" + String(CALIBRATION_SAMPLE_TIMES));
    }
    
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  
  // Calculate average and R0
  float sensorVoltageAvg = sensorVoltageSum / CALIBRATION_SAMPLE_TIMES;
  float rsAir = (3.3 - sensorVoltageAvg) / sensorVoltageAvg * RL_VALUE;
  calibratedRo = rsAir / RO_CLEAN_AIR_FACTOR;
  
  mq135Calibrated = true;
  
  Serial.print("Calibration complete. R0 = ");
  Serial.println(calibratedRo, 6);
  
  lcd.clear();
  lcd.print("Calibration OK");
  lcd.setCursor(0, 1);
  lcd.print("R0: " + String(calibratedRo, 4));
  delay(2000);
}

// =============================================
// WIFI CONNECTION
// =============================================
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  lcd.clear();
  lcd.print("WiFi: ");
  lcd.print(ssid);
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  
  WiFi.begin(ssid, pass);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink LED
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("\nWiFi connection failed!");
    lcd.clear();
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Using SMS Only");
    digitalWrite(LED_PIN, LOW);
  }
  
  delay(1000);
}

// =============================================
// BLYNK CONNECTION
// =============================================
void connectBlynk() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot connect to Blynk: No WiFi");
    blynkConnected = false;
    return;
  }
  
  Serial.println("Connecting to Blynk Cloud...");
  
  lcd.clear();
  lcd.print("Blynk Cloud");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  
  Blynk.config(auth);
  
  if (Blynk.connect(5000)) { // 5 second timeout
    blynkConnected = true;
    Serial.println("Blynk connected successfully!");
    
    lcd.clear();
    lcd.print("Blynk: Connected");
    
    // Send system startup notification
    Blynk.logEvent("system_start", "BINSAI Research System Online");
  } else {
    blynkConnected = false;
    Serial.println("Blynk connection failed!");
    
    lcd.clear();
    lcd.print("Blynk: Offline");
  }
  
  delay(1000);
}

// =============================================
// ULTRASONIC SENSOR READING
// =============================================
float readUltrasonicDistance() {
  // Ensure clean signal
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Send 10µs pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read echo with timeout (max 30ms for ~5m)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) {
    // Timeout or error
    return BIN_HEIGHT; // Assume empty
  }
  
  // Calculate distance (sound speed = 0.0343 cm/µs)
  float measuredDistance = duration * 0.0343 / 2.0;
  
  // Validate range (2cm - 400cm)
  if (measuredDistance < 2.0 || measuredDistance > 400.0) {
    return BIN_HEIGHT; // Invalid reading
  }
  
  return measuredDistance;
}

// =============================================
// MQ-135 SENSOR READING (WITH SCIENTIFIC CORRECTION)
// =============================================
float readMQ135PPM() {
  if (!mq135Calibrated) return 0.0;
  
  // Read analog value
  int sensorValue = analogRead(MQ135_PIN);
  float sensorVoltage = sensorValue * (3.3 / 4095.0);
  
  // Calculate RS
  float rsGas = (3.3 - sensorVoltage) / sensorVoltage * RL_VALUE;
  
  // Calculate ratio RS/R0
  float ratio = rsGas / calibratedRo;
  
  // Calculate PPM for CO₂ (from datasheet curve)
  // This is an approximation - for research, calibrate with known gas concentrations
  float ppm = 116.602068 * pow(ratio, -2.769034858);
  
  // Apply moving average filter (5 samples)
  static float ppmBuffer[5] = {0};
  static int bufferIndex = 0;
  
  ppmBuffer[bufferIndex] = ppm;
  bufferIndex = (bufferIndex + 1) % 5;
  
  // Calculate average
  float ppmAvg = 0;
  for (int i = 0; i < 5; i++) {
    ppmAvg += ppmBuffer[i];
  }
  ppmAvg /= 5.0;
  
  // Clamp to reasonable range
  ppmAvg = constrain(ppmAvg, 0, 2000);
  
  return ppmAvg;
}

// =============================================
// WASTE CLASSIFICATION ALGORITHM
// =============================================
void classifyWaste(float ppm, float fillPercent) {
  // Priority 0-3 system as per research design
  if (ppm <= PPM_NORMAL_MAX) {
    wasteType = "NORMAL";
    priorityLevel = 0;
    recommendation = "MONITORING";
  } 
  else if (ppm <= PPM_ANORGANIK_MAX) {
    wasteType = "ANORGANIK";
    priorityLevel = 1;
    recommendation = "Jadwal reguler";
  } 
  else if (ppm <= PPM_ORGANIK_L1_MAX) {
    wasteType = "ORGANIK L1";
    priorityLevel = 2;
    recommendation = "Siapkan kantong khusus";
  } 
  else {
    wasteType = "ORGANIK L2";
    priorityLevel = 3;
    recommendation = "SEGERA ANGKUT!";
  }
  
  // Update capacity status
  if (fillPercent <= 35) {
    capacityStatus = "KOSONG";
  } else if (fillPercent <= 50) {
    capacityStatus = "SETENGAH";
  } else if (fillPercent <= 90) {
    capacityStatus = "HAMPIR PENUH";
  } else {
    capacityStatus = "PENUH!";
  }
}

// =============================================
// GPS DATA PROCESSING
// =============================================
void processGPS() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
  
  if (gps.location.isUpdated() && gps.location.isValid()) {
    gpsLat = gps.location.lat();
    gpsLng = gps.location.lng();
    satellites = gps.satellites.value();
    
    // Consider GPS valid if we have at least 3 satellites
    gpsValid = (satellites >= 3);
    lastGPSFix = millis();
  }
  
  // If no update for 10 seconds, consider GPS invalid
  if (millis() - lastGPSFix > 10000) {
    gpsValid = false;
  }
}

// =============================================
// SMS STATE MACHINE (NON-BLOCKING)
// =============================================
void updateSMSStateMachine() {
  if (!sim800Ready || smsState == SMS_IDLE) return;
  
  unsigned long currentTime = millis();
  
  switch (smsState) {
    case SMS_PREPARE:
      // Prepare SMS message based on priority
      smsMessage = "[BINSAI RESEARCH ALERT]\n";
      smsMessage += "Status: " + capacityStatus + "\n";
      smsMessage += "Kapasitas: " + String((int)fillPercentage) + "%\n";
      smsMessage += "Jenis Sampah: " + wasteType + "\n";
      smsMessage += "Tingkat Gas: " + String((int)ppmValue) + " ppm\n";
      smsMessage += "Prioritas: " + String(priorityLevel) + "\n";
      smsMessage += "Rekomendasi: " + recommendation + "\n";
      
      if (gpsValid) {
        smsMessage += "Lokasi: https://maps.google.com/?q=";
        smsMessage += String(gpsLat, 6) + "," + String(gpsLng, 6);
      } else {
        smsMessage += "Lokasi: GPS sedang mencari sinyal";
      }
      
      smsRecipientIndex = 0;
      smsState = SMS_SENDING;
      smsStateTime = currentTime;
      break;
      
    case SMS_SENDING:
      if (smsRecipientIndex < phoneCount) {
        // Send AT command to initiate SMS
        sim800.print("AT+CMGS=\"");
        sim800.print(phoneNumbers[smsRecipientIndex]);
        sim800.println("\"");
        
        smsState = SMS_WAIT_CONFIRM;
        smsStateTime = currentTime;
      } else {
        // All recipients done
        smsState = SMS_IDLE;
      }
      break;
      
    case SMS_WAIT_CONFIRM:
      if (currentTime - smsStateTime > 1000) {
        // Wait for ">" prompt
        if (sim800.available()) {
          String response = sim800.readString();
          if (response.indexOf(">") != -1) {
            // Send message content
            sim800.print(smsMessage);
            delay(100);
            sim800.write(26); // Ctrl+Z to send
            delay(100);
            
            Serial.print("SMS sent to: ");
            Serial.println(phoneNumbers[smsRecipientIndex]);
            
            smsRecipientIndex++;
            smsState = SMS_SENDING;
            smsStateTime = currentTime;
          }
        }
        
        // Timeout after 5 seconds
        if (currentTime - smsStateTime > 5000) {
          Serial.println("SMS timeout, moving to next recipient");
          smsRecipientIndex++;
          smsState = SMS_SENDING;
          smsStateTime = currentTime;
        }
      }
      break;
  }
}

// =============================================
// CHECK AND TRIGGER SMS NOTIFICATION
// =============================================
void checkSMSNotification() {
  // Only send SMS if bin is >90% full AND cooldown period has passed
  if (fillPercentage > 90 && (millis() - lastSMSTime > SMS_COOLDOWN)) {
    
    // Check which priority level needs notification
    if (priorityLevel == 3 && !smsSentFlags[3]) {
      smsSentFlags[3] = true;
      smsState = SMS_PREPARE;
      lastSMSTime = millis();
      Serial.println("Triggering URGENT SMS notification");
    }
    else if (priorityLevel == 2 && !smsSentFlags[2]) {
      smsSentFlags[2] = true;
      smsState = SMS_PREPARE;
      lastSMSTime = millis();
      Serial.println("Triggering HIGH PRIORITY SMS");
    }
    else if (priorityLevel == 1 && !smsSentFlags[1]) {
      smsSentFlags[1] = true;
      smsState = SMS_PREPARE;
      lastSMSTime = millis();
      Serial.println("Triggering MEDIUM PRIORITY SMS");
    }
    else if (priorityLevel == 0 && !smsSentFlags[0]) {
      smsSentFlags[0] = true;
      smsState = SMS_PREPARE;
      lastSMSTime = millis();
      Serial.println("Triggering CAPACITY ALERT SMS");
    }
  }
  
  // Reset SMS flags if bin is emptied (<80%)
  if (fillPercentage < SMS_RESET_THRESHOLD) {
    for (int i = 0; i < 4; i++) {
      smsSentFlags[i] = false;
    }
  }
}

// =============================================
// UPDATE LCD DISPLAY
// =============================================
void updateDisplay() {
  static unsigned long lastDisplayUpdate = 0;
  
  if (millis() - lastDisplayUpdate < DISPLAY_ROTATE_INTERVAL) {
    return;
  }
  
  lastDisplayUpdate = millis();
  displayMode = (displayMode + 1) % 4;
  
  lcd.clear();
  
  switch (displayMode) {
    case 0: // Capacity View
      lcd.print("Kap: " + String((int)fillPercentage) + "%");
      lcd.setCursor(0, 1);
      lcd.print("Status: " + capacityStatus);
      break;
      
    case 1: // Waste Type View
      lcd.print("Jenis: " + wasteType);
      lcd.setCursor(0, 1);
      lcd.print("PPM: " + String((int)ppmValue));
      break;
      
    case 2: // Priority View
      lcd.print("Prioritas: " + String(priorityLevel));
      lcd.setCursor(0, 1);
      lcd.print(recommendation);
      break;
      
    case 3: // System Status View
      lcd.print("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "ON" : "OFF"));
      lcd.setCursor(0, 1);
      lcd.print("GPS: " + String(gpsValid ? "OK" : "NO"));
      break;
  }
}

// =============================================
// UPDATE BLYNK DATA
// =============================================
void updateBlynkData() {
  if (!blynkConnected) return;
  
  // Send all sensor data to Blynk
  Blynk.virtualWrite(V0, (int)fillPercentage);  // Fill percentage
  
  // LED Indicators (0=OFF, 255=ON)
  Blynk.virtualWrite(V1, (fillPercentage >= 90) ? 255 : 0);  // Red
  Blynk.virtualWrite(V2, (fillPercentage >= 70 && fillPercentage < 90) ? 255 : 0);  // Orange
  Blynk.virtualWrite(V3, (fillPercentage >= 40 && fillPercentage < 70) ? 255 : 0);  // Yellow
  Blynk.virtualWrite(V4, (fillPercentage < 40) ? 255 : 0);  // Green
  
  Blynk.virtualWrite(V5, distance);              // Distance in cm
  Blynk.virtualWrite(V6, capacityStatus);        // Status text
  
  Blynk.virtualWrite(V10, (int)ppmValue);        // PPM value
  Blynk.virtualWrite(V11, priorityLevel);        // Priority 0-3
  Blynk.virtualWrite(V12, wasteType);            // Waste type
  Blynk.virtualWrite(V13, recommendation);       // Recommendation
  
  // GPS data (only if valid)
  if (gpsValid) {
    Blynk.virtualWrite(V20, gpsLat);
    Blynk.virtualWrite(V21, gpsLng);
  }
  
  // Set color for V6 based on capacity status
  String colorCode;
  if (capacityStatus == "PENUH!") colorCode = "#FF0000";      // Red
  else if (capacityStatus == "HAMPIR PENUH") colorCode = "#FF9900"; // Orange
  else if (capacityStatus == "SETENGAH") colorCode = "#FFFF00";     // Yellow
  else colorCode = "#00FF00";                                  // Green
  
  Blynk.setProperty(V6, "color", colorCode);
}

// =============================================
// SYSTEM HEALTH CHECK
// =============================================
void checkSystemHealth() {
  Serial.println("\n=== SYSTEM HEALTH CHECK ===");
  Serial.print("WiFi: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.print("Blynk: ");
  Serial.println(blynkConnected ? "Connected" : "Disconnected");
  Serial.print("SIM800L: ");
  Serial.println(sim800Ready ? "Ready" : "Not Ready");
  Serial.print("GPS: ");
  Serial.println(gpsValid ? "Valid Fix" : "No Fix");
  Serial.print("Satellites: ");
  Serial.println(satellites);
  Serial.print("MQ-135 Calibrated: ");
  Serial.println(mq135Calibrated ? "Yes" : "No");
  Serial.println("===========================");
  
  // Log to Blynk if connected
  if (blynkConnected) {
    String healthMsg = "System OK";
    if (!gpsValid) healthMsg += " | GPS Searching";
    if (!sim800Ready) healthMsg += " | GSM Offline";
    Blynk.logEvent("system_health", healthMsg);
  }
}

// =============================================
// RECONNECTION ATTEMPT
// =============================================
void attemptReconnections() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting WiFi reconnection...");
    connectWiFi();
  }
  
  if (WiFi.status() == WL_CONNECTED && !blynkConnected) {
    Serial.println("Attempting Blynk reconnection...");
    connectBlynk();
  }
  
  // Check SIM800L status
  if (!digitalRead(SIM800_STATUS)) { // Typically active LOW
    Serial.println("SIM800L appears offline, attempting restart...");
    initSIM800L();
  }
}

// =============================================
// MAIN LOOP
// =============================================
void loop() {
  // Run Blynk (non-blocking)
  if (blynkConnected) {
    Blynk.run();
  }
  
  // Run timers
  timer.run();
  
  // Process GPS data
  processGPS();
  
  // Update sensors at regular interval
  if (millis() - lastUpdateTime >= UPDATE_INTERVAL) {
    lastUpdateTime = millis();
    
    // Read sensors
    distance = readUltrasonicDistance();
    fillPercentage = (1.0 - (distance / BIN_HEIGHT)) * 100.0;
    fillPercentage = constrain(fillPercentage, 0, 100);
    
    ppmValue = readMQ135PPM();
    
    // Classify waste
    classifyWaste(ppmValue, fillPercentage);
    
    // Check for SMS notifications
    checkSMSNotification();
    
    // Update display
    updateDisplay();
    
    // Print to serial for debugging
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print("cm | Fill: ");
    Serial.print(fillPercentage);
    Serial.print("% | PPM: ");
    Serial.print(ppmValue);
    Serial.print(" | Type: ");
    Serial.print(wasteType);
    Serial.print(" | Priority: ");
    Serial.println(priorityLevel);
  }
  
  // Process SMS state machine
  updateSMSStateMachine();
  
  // Check for emergency button press (reset credentials)
  static unsigned long buttonPressStart = 0;
  if (digitalRead(BUTTON_PIN) == LOW) { // Button pressed (LOW with pullup)
    if (buttonPressStart == 0) {
      buttonPressStart = millis();
    } else if (millis() - buttonPressStart > 5000) { // 5 second hold
      Serial.println("Factory reset initiated!");
      lcd.clear();
      lcd.print("Factory Reset");
      lcd.setCursor(0, 1);
      lcd.print("Hold 5s more...");
      
      if (millis() - buttonPressStart > 10000) { // 10 second total
        prefs.begin(PREF_NAMESPACE, false);
        prefs.clear();
        prefs.end();
        
        lcd.clear();
        lcd.print("Reset Complete");
        lcd.setCursor(0, 1);
        lcd.print("Restarting...");
        
        delay(2000);
        ESP.restart();
      }
    }
  } else {
    buttonPressStart = 0;
  }
}

// =============================================
// BLYNK APP WIDGET CONFIGURATION GUIDE
// =============================================
/*
BLYNK DASHBOARD SETUP:

1. GAUGE WIDGET (Fill Percentage):
   - Pin: V0
   - Label: "Kapasitas Bak"
   - Range: 0-100
   - Color: Green(0-50), Yellow(50-80), Red(80-100)

2. LED INDICATORS (4x):
   - V1: LED Red (Pin: V1, Label: "PENUH")
   - V2: LED Orange (Pin: V2, Label: "HAMPIR PENUH")
   - V3: LED Yellow (Pin: V3, Label: "SETENGAH")
   - V4: LED Green (Pin: V4, Label: "KOSONG")

3. VALUE DISPLAY WIDGETS:
   - Distance: Pin V5, Label: "Jarak", Suffix: "cm"
   - Status: Pin V6, Label: "Status Kapasitas"
   - PPM: Pin V10, Label: "Tingkat Gas", Suffix: "ppm"
   - Priority: Pin V11, Label: "Tingkat Prioritas"
   - Waste Type: Pin V12, Label: "Jenis Sampah"
   - Recommendation: Pin V13, Label: "Rekomendasi"

4. MAP WIDGET:
   - Latitude: V20
   - Longitude: V21
   - Label: "Lokasi Bak Sampah"

5. SUPERTABLE (Historical Data):
   - Add V0 (Fill %), V10 (PPM), V11 (Priority)
   - Time range: 24 hours
   - Auto-refresh: 2 seconds

6. NOTIFICATION SETTINGS:
   - Configure Blynk app notifications
   - Set threshold alerts for V0 > 90%
   - Set priority alerts for V11 > 2
*/

// =============================================
// RESEARCH DATA LOGGING FUNCTIONS
// =============================================
void logResearchData() {
  // Untuk penelitian, log data ke SD card atau cloud database
  static unsigned long lastLogTime = 0;
  
  if (millis() - lastLogTime > 60000) { // Log setiap 1 menit
    lastLogTime = millis();
    
    String dataLog = String(millis()) + ",";
    dataLog += String(fillPercentage, 1) + ",";
    dataLog += String(ppmValue, 1) + ",";
    dataLog += String(priorityLevel) + ",";
    dataLog += wasteType + ",";
    dataLog += String(gpsValid ? 1 : 0) + ",";
    dataLog += String(gpsLat, 6) + ",";
    dataLog += String(gpsLng, 6);
    
    Serial.println("RESEARCH_DATA: " + dataLog);
    
    // Untuk implementasi lengkap, tambahkan SD card logging
    // atau upload ke Google Sheets/ThingSpeak
  }
}
