// =============================================
// SMART WASTE MONITORING SYSTEM
// ESP32 + HC-SR04 + LCD I2C + Debug Monitoring
// =============================================

// Libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============ PIN DEFINITION (ESP32) ============
// HC-SR04 Ultrasonic Sensor
#define TRIG_PIN 5     // GPIO5 - Output
#define ECHO_PIN 18    // GPIO18 - Input (5V tolerant)

// LCD I2C using default ESP32 pins
// SDA = GPIO21, SCL = GPIO22

// ============ SYSTEM PARAMETERS ============
#define BIN_MAX_HEIGHT 40.0     // Bin height in cm
#define UPDATE_INTERVAL 2000    // Update data every 2 seconds
#define SAMPLE_COUNT 5          // Number of samples for filtering
#define SENSOR_TIMEOUT 30000    // Sensor timeout 30ms (~5m)

// ============ STATUS THRESHOLDS ============
#define EMPTY_MAX 35.0
#define HALF_MIN 36.0
#define HALF_MAX 50.0
#define ALMOST_FULL_MIN 51.0
#define ALMOST_FULL_MAX 90.0
#define FULL_MIN 91.0

// ============ GLOBAL VARIABLES ============
float measuredDistance = 0.0;            // Distance in cm
float fillPercentage = 0.0;              // Percentage 0-100%
String capacityStatus = "MEASURING";     // Textual status
unsigned long lastUpdateTime = 0;
int lcdScreen = 0;                       // 0=Distance&Percentage, 1=Status, 2=System Info
unsigned long screenChangeTime = 0;
const int screenChangeInterval = 3000;   // Change screen every 3 seconds

// Error tracking variables
int sensorErrorCount = 0;
int lcdErrorCount = 0;
String lastError = "NONE";
unsigned long systemUptime = 0;
unsigned long lastErrorTime = 0;

// ============ OBJECTS ============
// Initialize LCD with address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =============================================
// SETUP FUNCTION
// =============================================
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n" + String(78, '='));
  Serial.println("       SMART WASTE MONITORING SYSTEM");
  Serial.println("           ESP32 + HC-SR04 + LCD I2C");
  Serial.println(String(78, '='));
  
  // Initialization sequence
  Serial.println("\n[1/4] Initializing pins...");
  initializePins();
  
  Serial.println("[2/4] Initializing LCD I2C...");
  initializeLCD();
  
  Serial.println("[3/4] Checking system health...");
  performSystemDiagnostics();
  
  Serial.println("[4/4] Starting monitoring system...");
  
  // Display startup completion
  Serial.println("\n✅ SYSTEM READY FOR OPERATION!");
  Serial.println("📏 Bin Height: " + String(BIN_MAX_HEIGHT) + " cm");
  Serial.println("📊 Thresholds: Empty(0-35%), Half(36-50%)");
  Serial.println("                Almost Full(51-90%), Full(91-100%)");
  Serial.println(String(78, '=') + "\n");
  
  // LCD welcome message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  lcd.setCursor(0, 1);
  lcd.print("Height: ");
  lcd.print(BIN_MAX_HEIGHT);
  lcd.print(" cm");
  delay(2000);
}

// =============================================
// PIN INITIALIZATION
// =============================================
void initializePins() {
  Serial.println("   Configuring HC-SR04:");
  Serial.println("   - TRIG_PIN: GPIO" + String(TRIG_PIN) + " (OUTPUT)");
  Serial.println("   - ECHO_PIN: GPIO" + String(ECHO_PIN) + " (INPUT)");
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  
  Serial.println("   ✅ Pins configured successfully");
}

// =============================================
// LCD I2C INITIALIZATION
// =============================================
void initializeLCD() {
  Serial.println("   Configuring LCD I2C:");
  Serial.println("   - Address: 0x27");
  Serial.println("   - SDA: GPIO21 (default ESP32)");
  Serial.println("   - SCL: GPIO22 (default ESP32)");
  
  // Start I2C with default pins
  Wire.begin();
  delay(100);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Try to detect LCD at address 0x27
  Wire.beginTransmission(0x27);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("   ✅ LCD detected at address 0x27");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD Connected");
    lcd.setCursor(0, 1);
    lcd.print("Address: 0x27");
  } else {
    Serial.println("   ⚠️  LCD not detected at 0x27!");
    Serial.println("   Trying alternative addresses...");
    
    // Try common alternative addresses
    byte addresses[] = {0x3F, 0x27, 0x20, 0x38};
    bool found = false;
    
    for (int i = 0; i < 4; i++) {
      Wire.beginTransmission(addresses[i]);
      error = Wire.endTransmission();
      if (error == 0) {
        Serial.print("   ✅ LCD found at address: 0x");
        Serial.println(addresses[i], HEX);
        
        // Reinitialize with found address
        lcd = LiquidCrystal_I2C(addresses[i], 16, 2);
        lcd.init();
        lcd.backlight();
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("LCD Found");
        lcd.setCursor(0, 1);
        lcd.print("Addr: 0x");
        lcd.print(addresses[i], HEX);
        
        found = true;
        break;
      }
    }
    
    if (!found) {
      Serial.println("   ❌ LCD not found!");
      Serial.println("   Check I2C connections:");
      Serial.println("   - SDA -> GPIO21");
      Serial.println("   - SCL -> GPIO22");
      Serial.println("   - VCC -> 5V");
      Serial.println("   - GND -> GND");
      logError("LCD_NOT_FOUND", "I2C connection failed");
    }
  }
  
  delay(1500);
}

// =============================================
// SYSTEM DIAGNOSTICS
// =============================================
void performSystemDiagnostics() {
  Serial.println("\n=== SYSTEM DIAGNOSTICS ===");
  
  // Test ultrasonic sensor
  Serial.print("Testing HC-SR04... ");
  float testDistance = readDistance();
  if (testDistance > 0 && testDistance < 200) {
    Serial.println("OK (" + String(testDistance, 1) + " cm)");
  } else {
    Serial.println("FAILED!");
    logError("ULTRASONIC_FAIL", "Invalid reading: " + String(testDistance));
  }
  
  // Test LCD
  Serial.print("Testing LCD... ");
  lcd.clear();
  lcd.print("Diagnostic Test");
  delay(500);
  if (checkLCDFunction()) {
    Serial.println("OK");
  } else {
    Serial.println("FAILED");
    logError("LCD_TEST_FAIL", "Display malfunction");
  }
  
  // Check memory
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
  
  Serial.println("=== DIAGNOSTICS COMPLETE ===\n");
}

// =============================================
// READ HC-SR04 SENSOR
// =============================================
float readDistance() {
  float totalDistance = 0;
  int validReadings = 0;
  
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    // Trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // Read echo with timeout
    long duration = pulseIn(ECHO_PIN, HIGH, SENSOR_TIMEOUT);
    
    if (duration > 0) {
      float currentDistance = duration * 0.0343 / 2.0; // Sound speed 343 m/s
      
      // Filter valid range (2cm - 200cm)
      if (currentDistance >= 2.0 && currentDistance <= 200.0) {
        totalDistance += currentDistance;
        validReadings++;
      } else {
        logError("SENSOR_RANGE", "Invalid range: " + String(currentDistance));
      }
    } else {
      // Timeout occurred
      logError("SENSOR_TIMEOUT", "PulseIn timeout on attempt " + String(i+1));
    }
    delay(50);
  }
  
  if (validReadings > 0) {
    float averageDistance = totalDistance / validReadings;
    
    // Limit to bin height
    if (averageDistance > BIN_MAX_HEIGHT) {
      averageDistance = BIN_MAX_HEIGHT;
    }
    
    // Reset error count on successful reading
    if (sensorErrorCount > 0) sensorErrorCount = 0;
    
    return averageDistance;
  } else {
    // No valid readings
    sensorErrorCount++;
    logError("NO_VALID_READINGS", "All " + String(SAMPLE_COUNT) + " readings failed");
    return BIN_MAX_HEIGHT; // Assume empty if no readings
  }
}

// =============================================
// CALCULATE FILL PERCENTAGE
// =============================================
void calculateFillPercentage() {
  // Formula: (1 - Distance/Height) × 100%
  if (measuredDistance >= BIN_MAX_HEIGHT) {
    fillPercentage = 0.0;
  } else {
    fillPercentage = (1.0 - (measuredDistance / BIN_MAX_HEIGHT)) * 100.0;
  }
  
  // Clamp to 0-100%
  if (fillPercentage < 0.0) fillPercentage = 0.0;
  if (fillPercentage > 100.0) fillPercentage = 100.0;
  
  // Classify status according to thresholds
  if (fillPercentage <= EMPTY_MAX) {
    capacityStatus = "EMPTY";
  } else if (fillPercentage >= HALF_MIN && fillPercentage <= HALF_MAX) {
    capacityStatus = "HALF";
  } else if (fillPercentage >= ALMOST_FULL_MIN && fillPercentage <= ALMOST_FULL_MAX) {
    capacityStatus = "ALMOST FULL";
  } else if (fillPercentage >= FULL_MIN) {
    capacityStatus = "FULL";
  }
}

// =============================================
// UPDATE LCD DISPLAY
// =============================================
void updateLCD() {
  // Change screen every interval
  if (millis() - screenChangeTime >= screenChangeInterval) {
    screenChangeTime = millis();
    lcdScreen = (lcdScreen + 1) % 4; // 4 screens total
  }
  
  lcd.clear();
  
  switch (lcdScreen) {
    case 0:
      // Screen 1: Distance & Percentage
      lcd.setCursor(0, 0);
      lcd.print("Dist:");
      lcd.print(measuredDistance, 1);
      lcd.print(" cm");
      
      lcd.setCursor(0, 1);
      lcd.print("Fill: ");
      lcd.print(fillPercentage, 1);
      lcd.print(" %");
      break;
      
    case 1:
      // Screen 2: Capacity Status
      lcd.setCursor(0, 0);
      lcd.print("Bin Status:");
      
      lcd.setCursor(0, 1);
      // Center the status
      int spaces = (16 - capacityStatus.length()) / 2;
      if (spaces > 0) {
        lcd.setCursor(spaces, 1);
      }
      lcd.print(capacityStatus);
      break;
      
    case 2:
      // Screen 3: System Information
      lcd.setCursor(0, 0);
      lcd.print("Uptime: ");
      lcd.print(systemUptime / 3600); // Hours
      lcd.print("h");
      
      lcd.setCursor(0, 1);
      lcd.print("Heap: ");
      lcd.print(ESP.getFreeHeap() / 1024);
      lcd.print(" KB");
      break;
      
    case 3:
      // Screen 4: Error Information (if any)
      if (lastError != "NONE") {
        lcd.setCursor(0, 0);
        lcd.print("Last Error:");
        
        lcd.setCursor(0, 1);
        String errorDisplay = lastError.substring(0, 16);
        lcd.print(errorDisplay);
      } else {
        lcd.setCursor(0, 0);
        lcd.print("No Errors");
        lcd.setCursor(0, 1);
        lcd.print("System OK");
      }
      break;
  }
}

// =============================================
// CHECK LCD FUNCTION
// =============================================
bool checkLCDFunction() {
  // Simple LCD functionality test
  lcd.clear();
  lcd.print("Test");
  delay(100);
  
  // Check if we can write and clear
  lcd.clear();
  lcd.print("1234567890123456");
  delay(100);
  
  String testLine = "";
  for (int i = 0; i < 16; i++) {
    testLine += char('A' + (i % 26));
  }
  
  lcd.setCursor(0, 1);
  lcd.print(testLine);
  delay(500);
  
  lcd.clear();
  return true;
}

// =============================================
// LOG ERROR FUNCTION
// =============================================
void logError(String errorCode, String errorMessage) {
  lastError = errorCode + ": " + errorMessage;
  lastErrorTime = millis();
  
  Serial.println("[ERROR] " + errorCode + " - " + errorMessage);
  Serial.println("  Time: " + String(millis() / 1000) + "s");
  Serial.println("  Heap: " + String(ESP.getFreeHeap()) + " bytes");
}

// =============================================
// PRINT SENSOR DATA TO SERIAL
// =============================================
void printSensorData() {
  Serial.println("\n" + String(60, '='));
  Serial.println("           HC-SR04 SENSOR DATA");
  Serial.println(String(60, '='));
  
  Serial.println("📏 Raw Data:");
  Serial.print("   Measured Distance: ");
  Serial.print(measuredDistance, 1);
  Serial.println(" cm");
  
  Serial.println("\n📊 Fill Calculation:");
  Serial.print("   Percentage: ");
  Serial.print(fillPercentage, 1);
  Serial.println(" %");
  Serial.print("   Status: ");
  Serial.println(capacityStatus);
  
  Serial.println("\n📈 Bin Information:");
  Serial.print("   Max Height: ");
  Serial.print(BIN_MAX_HEIGHT, 1);
  Serial.println(" cm");
  Serial.print("   Filled Height: ");
  Serial.print(BIN_MAX_HEIGHT - measuredDistance, 1);
  Serial.println(" cm");
  
  if (sensorErrorCount > 0) {
    Serial.println("\n⚠️  Warnings:");
    Serial.print("   Sensor Errors: ");
    Serial.println(sensorErrorCount);
  }
  
  if (lastError != "NONE") {
    Serial.print("   Last Error: ");
    Serial.println(lastError);
  }
  
  Serial.println(String(60, '=') + "\n");
}

// =============================================
// PRINT SYSTEM STATUS
// =============================================
void printSystemStatus() {
  static unsigned long lastStatusPrint = 0;
  
  if (millis() - lastStatusPrint >= 10000) { // Every 10 seconds
    lastStatusPrint = millis();
    
    Serial.println("[SYSTEM STATUS]");
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap() / 1024);
    Serial.println(" KB");
    
    Serial.print("Min Free Heap: ");
    Serial.print(ESP.getMinFreeHeap() / 1024);
    Serial.println(" KB");
    
    Serial.print("Sensor Errors: ");
    Serial.println(sensorErrorCount);
    
    Serial.print("LCD Errors: ");
    Serial.println(lcdErrorCount);
    
    if (lastError != "NONE") {
      Serial.print("Last Error: ");
      Serial.println(lastError);
      Serial.print("Error Age: ");
      Serial.print((millis() - lastErrorTime) / 1000);
      Serial.println(" seconds");
    }
    
    Serial.println("----------------------------");
  }
}

// =============================================
// MAIN LOOP
// =============================================
void loop() {
  // Update system uptime
  systemUptime = millis() / 1000;
  
  // Update sensor at specified interval
  if (millis() - lastUpdateTime >= UPDATE_INTERVAL) {
    lastUpdateTime = millis();
    
    Serial.println("\n🔄 Reading HC-SR04 sensor...");
    
    // Read distance (inverted for fill calculation)
    measuredDistance = 40 - readDistance(); // Inverted: 0 = empty, 40 = full
    
    // Calculate percentage and status
    calculateFillPercentage();
    
    // Update LCD
    updateLCD();
    
    // Print to serial monitor
    printSensorData();
    
    // Check for critical conditions
    if (fillPercentage >= FULL_MIN) {
      Serial.println("🚨 CRITICAL: Bin is FULL! Immediate attention required!");
      // Here you could trigger an alarm or notification
    } else if (fillPercentage >= ALMOST_FULL_MIN) {
      Serial.println("⚠️  WARNING: Bin is ALMOST FULL. Schedule collection soon.");
    }
  }
  
  // Print system status periodically
  printSystemStatus();
  
  // Check for error conditions that need recovery
  if (sensorErrorCount >= 10) {
    Serial.println("🔧 Attempting sensor recovery...");
    // Reset sensor pins
    digitalWrite(TRIG_PIN, LOW);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    sensorErrorCount = 0;
    delay(1000);
  }
  
  // Small delay to prevent watchdog issues
  delay(100);
}
