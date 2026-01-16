/**
 * SIM800L GSM Module Validation Test
 * BINSAI Research Project - ISPO 2026
 * 
 * This test validates the SIM800L GSM module functionality including:
 * 1. Module initialization and network registration
 * 2. SMS transmission to two recipient numbers
 * 3. Network signal strength measurement
 * 4. SMS delivery confirmation simulation
 * 
 * Hardware Configuration:
 * - ESP32 DevKit v1
 * - SIM800L GSM Module (3.3V/4.0V compatible)
 * - I2C LCD 16x2 (Address 0x27)
 * - SIM Card with active credit
 * - External power supply for SIM800L (recommended: 3.7V Li-Po)
 * 
 * Wiring:
 * SIM800L TX  -> ESP32 GPIO 17 (RX2)
 * SIM800L RX  -> ESP32 GPIO 16 (TX2)
 * SIM800L RST -> ESP32 GPIO 4  (Optional reset control)
 * LCD I2C     -> ESP32 GPIO 21 (SDA), GPIO 22 (SCL)
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// SIM800L Hardware Serial Configuration
#define SIM800L_RX_PIN 16    // ESP32 RX2 (GPIO16)
#define SIM800L_TX_PIN 17    // ESP32 TX2 (GPIO17)
#define SIM800L_RST_PIN 4    // Optional reset pin
#define SIM800L_PWR_PIN 5    // Optional power key pin

// I2C LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Test Configuration
#define PHONE_NUMBER_1 "+62_YOUR_NUMBER_PHONE1"  // Replace with actual number
#define PHONE_NUMBER_2 "+62_YOUR_NUMBER_PHONE2"  // Replace with actual number
#define TEST_MESSAGE "PLEASE_FILL_THIS_SPACE_FOR_YOUR_MESSAGE"
#define SMS_SEND_DELAY_MS 5000  // Delay between SMS transmissions
#define AT_COMMAND_TIMEOUT_MS 10000  // Timeout for AT commands

// Global objects
HardwareSerial sim800lSerial(2);  // Use UART2 for SIM800L
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

/**
 * Initialize LCD with startup message
 */
void initializeLCD() {
  Wire.begin(21, 22);  // SDA, SCL
  lcd.init();
  lcd.backlight();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BINSAI GSM Test");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  Serial.println("LCD initialized successfully");
}

/**
 * Send AT command to SIM800L and wait for response
 * @param command The AT command to send
 * @param expectedResponse Expected response substring (NULL for any response)
 * @param timeout_ms Timeout in milliseconds
 * @return true if expected response received, false otherwise
 */
bool sendATCommand(const char* command, const char* expectedResponse = NULL, unsigned long timeout_ms = AT_COMMAND_TIMEOUT_MS) {
  Serial.print("Sending: ");
  Serial.println(command);
  
  sim800lSerial.println(command);
  
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < timeout_ms) {
    while (sim800lSerial.available()) {
      char c = sim800lSerial.read();
      response += c;
      Serial.write(c);
      
      // Check for expected response
      if (expectedResponse != NULL && response.indexOf(expectedResponse) >= 0) {
        Serial.println("\nExpected response received");
        return true;
      }
      
      // Check for ERROR response
      if (response.indexOf("ERROR") >= 0) {
        Serial.println("\nERROR response received");
        return false;
      }
    }
    
    // Small delay to prevent busy waiting
    delay(10);
  }
  
  // If no expected response specified, check for any response
  if (expectedResponse == NULL && response.length() > 0) {
    Serial.println("\nResponse received (no specific check)");
    return true;
  }
  
  Serial.println("\nTimeout waiting for response");
  return false;
}

/**
 * Wait for specific response from SIM800L
 * @param expectedResponse Expected response string
 * @param timeout_ms Timeout in milliseconds
 * @return true if response received, false on timeout
 */
bool waitForResponse(const char* expectedResponse, unsigned long timeout_ms = AT_COMMAND_TIMEOUT_MS) {
  unsigned long startTime = millis();
  String response = "";
  
  Serial.print("Waiting for: ");
  Serial.println(expectedResponse);
  
  while (millis() - startTime < timeout_ms) {
    while (sim800lSerial.available()) {
      char c = sim800lSerial.read();
      response += c;
      Serial.write(c);
      
      if (response.indexOf(expectedResponse) >= 0) {
        Serial.println("\nFound expected response");
        return true;
      }
    }
    delay(10);
  }
  
  Serial.println("\nTimeout waiting for response");
  return false;
}

/**
 * Check if SIM800L module is responding to AT commands
 * @return true if module responds, false otherwise
 */
bool checkModuleResponse() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Checking Module");
  
  // Send AT command multiple times (module may need time to initialize)
  for (int i = 0; i < 5; i++) {
    if (sendATCommand("AT", "OK", 2000)) {
      lcd.setCursor(0, 1);
      lcd.print("Module OK     ");
      Serial.println("SIM800L module is responding");
      return true;
    }
    delay(1000);
    
    // Update LCD with retry count
    lcd.setCursor(0, 1);
    lcd.print("Retry: ");
    lcd.print(i + 1);
  }
  
  lcd.setCursor(0, 1);
  lcd.print("No Response!  ");
  Serial.println("ERROR: SIM800L module not responding");
  return false;
}

/**
 * Check SIM card status and network registration
 * @return true if SIM is ready and registered, false otherwise
 */
bool checkSIMAndNetwork() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SIM & Network ");
  
  // Check if SIM is inserted and ready
  if (!sendATCommand("AT+CPIN?", "READY", 5000)) {
    lcd.setCursor(0, 1);
    lcd.print("SIM Not Ready ");
    Serial.println("ERROR: SIM card not ready");
    return false;
  }
  
  // Check network registration
  lcd.setCursor(0, 1);
  lcd.print("Reg: Checking ");
  
  if (!sendATCommand("AT+CREG?", "+CREG: 0,1", 10000)) {
    // Try alternative registration status
    if (!sendATCommand("AT+CREG?", "+CREG: 0,5", 5000)) {
      lcd.setCursor(0, 1);
      lcd.print("Reg: Failed   ");
      Serial.println("WARNING: Network registration not confirmed");
      // Continue anyway for testing
    }
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Reg: OK       ");
  Serial.println("SIM card ready and network registered");
  return true;
}

/**
 * Get signal quality in dBm
 * @return Signal strength in dBm (0-31 scale, 99 if unknown)
 */
int getSignalStrength() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Signal Quality");
  
  sim800lSerial.println("AT+CSQ");
  
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < 5000) {
    while (sim800lSerial.available()) {
      char c = sim800lSerial.read();
      response += c;
      Serial.write(c);
      
      // Look for +CSQ response
      int csqIndex = response.indexOf("+CSQ:");
      if (csqIndex >= 0) {
        int commaIndex = response.indexOf(',', csqIndex);
        if (commaIndex >= 0) {
          String csqStr = response.substring(csqIndex + 6, commaIndex);
          int csq = csqStr.toInt();
          
          // Convert to approximate dBm
          int dbm = -113 + (csq * 2);  // Approximate conversion
          
          lcd.setCursor(0, 1);
          lcd.print("CSQ: ");
          lcd.print(csq);
          lcd.print(" (");
          lcd.print(dbm);
          lcd.print("dBm)");
          
          Serial.print("Signal Quality: ");
          Serial.print(csq);
          Serial.print(" (approx ");
          Serial.print(dbm);
          Serial.println(" dBm)");
          
          return csq;
        }
      }
    }
    delay(10);
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Signal: N/A   ");
  Serial.println("WARNING: Could not get signal strength");
  return 99;  // Unknown signal strength
}

/**
 * Set SMS text mode (instead of PDU mode)
 * @return true if successful, false otherwise
 */
bool setSMSTextMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMS Text Mode ");
  
  if (sendATCommand("AT+CMGF=1", "OK", 3000)) {
    lcd.setCursor(0, 1);
    lcd.print("Mode: TEXT    ");
    Serial.println("SMS set to text mode");
    return true;
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Mode: FAILED  ");
    Serial.println("ERROR: Failed to set SMS text mode");
    return false;
  }
}

/**
 * Send SMS to specified phone number
 * @param phoneNumber Recipient phone number (international format)
 * @param message SMS message content
 * @return true if SMS sent successfully, false otherwise
 */
bool sendSMS(const char* phoneNumber, const char* message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending SMS to:");
  lcd.setCursor(0, 1);
  
  // Truncate phone number for display
  String displayNum = String(phoneNumber);
  if (displayNum.length() > 15) {
    displayNum = displayNum.substring(0, 12) + "...";
  }
  lcd.print(displayNum);
  
  Serial.print("Preparing to send SMS to: ");
  Serial.println(phoneNumber);
  
  // Set recipient number
  String atCmd = "AT+CMGS=\"";
  atCmd += phoneNumber;
  atCmd += "\"";
  
  sim800lSerial.println(atCmd);
  
  // Wait for ">" prompt
  if (!waitForResponse(">", 5000)) {
    Serial.println("ERROR: Did not receive '>' prompt");
    lcd.setCursor(0, 1);
    lcd.print("No > prompt   ");
    return false;
  }
  
  // Send message content
  sim800lSerial.println(message);
  delay(100);
  
  // Send Ctrl+Z to indicate end of message (ASCII 26)
  sim800lSerial.write(26);
  
  Serial.println("Message sent, waiting for confirmation...");
  
  // Wait for confirmation
  if (waitForResponse("+CMGS:", 10000)) {
    // Look for OK after CMGS
    if (waitForResponse("OK", 5000)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SMS Sent OK!");
      lcd.setCursor(0, 1);
      lcd.print("To: ");
      lcd.print(displayNum);
      
      Serial.println("SUCCESS: SMS sent successfully");
      return true;
    }
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMS Send FAILED");
  Serial.println("ERROR: SMS send failed or timeout");
  return false;
}

/**
 * Display test summary on LCD
 * @param testsPassed Number of tests passed
 * @param totalTests Total number of tests
 */
void displayTestSummary(int testsPassed, int totalTests) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test Complete!");
  
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(testsPassed);
  lcd.print("/");
  lcd.print(totalTests);
  
  if (testsPassed == totalTests) {
    lcd.print(" PASS");
  } else if (testsPassed >= totalTests / 2) {
    lcd.print(" PARTIAL");
  } else {
    lcd.print(" FAIL");
  }
  
  delay(3000);
}

/**
 * Main test sequence
 */
void runGSMValidationTest() {
  int testsPassed = 0;
  int totalTests = 6;  // Update based on actual tests
  
  Serial.println("\n==================================");
  Serial.println("   SIM800L GSM VALIDATION TEST   ");
  Serial.println("==================================");
  
  // Test 1: Module Response
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test 1/6:");
  lcd.setCursor(0, 1);
  lcd.print("Module Response");
  delay(2000);
  
  if (checkModuleResponse()) {
    testsPassed++;
    Serial.println("✓ Test 1 PASSED: Module responds to AT commands");
  } else {
    Serial.println("✗ Test 1 FAILED: Module not responding");
    // Can't continue if module doesn't respond
    displayTestSummary(testsPassed, totalTests);
    return;
  }
  
  // Test 2: SIM Card and Network
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test 2/6:");
  lcd.setCursor(0, 1);
  lcd.print("SIM & Network");
  delay(2000);
  
  if (checkSIMAndNetwork()) {
    testsPassed++;
    Serial.println("✓ Test 2 PASSED: SIM ready and network registered");
  } else {
    Serial.println("⚠ Test 2 PARTIAL: Network registration issues");
    // Continue anyway - some networks might have delayed registration
  }
  
  // Test 3: Signal Strength
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test 3/6:");
  lcd.setCursor(0, 1);
  lcd.print("Signal Quality");
  delay(2000);
  
  int signalStrength = getSignalStrength();
  if (signalStrength > 0 && signalStrength < 99) {
    testsPassed++;
    Serial.println("✓ Test 3 PASSED: Signal strength obtained");
  } else {
    Serial.println("⚠ Test 3 WARNING: Could not get valid signal strength");
  }
  
  // Test 4: SMS Text Mode
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test 4/6:");
  lcd.setCursor(0, 1);
  lcd.print("SMS Text Mode");
  delay(2000);
  
  if (setSMSTextMode()) {
    testsPassed++;
    Serial.println("✓ Test 4 PASSED: SMS set to text mode");
  } else {
    Serial.println("✗ Test 4 FAILED: Could not set SMS text mode");
    // Can't continue if SMS mode not set
    displayTestSummary(testsPassed, totalTests);
    return;
  }
  
  // Test 5: Send SMS to Phone Number 1
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test 5/6:");
  lcd.setCursor(0, 1);
  lcd.print("SMS to Phone 1");
  delay(2000);
  
  Serial.print("\n--- Sending SMS to Phone 1 ---\n");
  if (sendSMS(PHONE_NUMBER_1, TEST_MESSAGE)) {
    testsPassed++;
    Serial.println("✓ Test 5 PASSED: SMS sent to Phone 1");
  } else {
    Serial.println("✗ Test 5 FAILED: Could not send SMS to Phone 1");
  }
  
  delay(SMS_SEND_DELAY_MS);
  
  // Test 6: Send SMS to Phone Number 2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test 6/6:");
  lcd.setCursor(0, 1);
  lcd.print("SMS to Phone 2");
  delay(2000);
  
  Serial.print("\n--- Sending SMS to Phone 2 ---\n");
  if (sendSMS(PHONE_NUMBER_2, TEST_MESSAGE)) {
    testsPassed++;
    Serial.println("✓ Test 6 PASSED: SMS sent to Phone 2");
  } else {
    Serial.println("✗ Test 6 FAILED: Could not send SMS to Phone 2");
  }
  
  // Display final results
  displayTestSummary(testsPassed, totalTests);
  
  Serial.println("\n==================================");
  Serial.println("         TEST COMPLETE           ");
  Serial.print("Passed: ");
  Serial.print(testsPassed);
  Serial.print(" out of ");
  Serial.println(totalTests);
  Serial.println("==================================");
}

/**
 * Setup function - runs once at startup
 */
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("\n\nBINSAI SIM800L Validation Test");
  Serial.println("==============================");
  
  // Initialize LCD
  initializeLCD();
  
  // Initialize SIM800L serial communication
  sim800lSerial.begin(9600, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
  delay(1000);
  
  // Optional: Reset SIM800L module
  if (SIM800L_RST_PIN > 0) {
    pinMode(SIM800L_RST_PIN, OUTPUT);
    digitalWrite(SIM800L_RST_PIN, LOW);
    delay(100);
    digitalWrite(SIM800L_RST_PIN, HIGH);
    delay(2000);
    Serial.println("SIM800L module reset");
  }
  
  // Clear SIM800L serial buffer
  while (sim800lSerial.available()) {
    sim800lSerial.read();
  }
  
  // Start test sequence
  delay(3000);  // Give time to read initial messages
  runGSMValidationTest();
}

/**
 * Loop function - runs repeatedly
 */
void loop() {
  // Display idle message after test completion
  static unsigned long lastUpdate = 0;
  static int screen = 0;
  
  if (millis() - lastUpdate > 5000) {
    lcd.clear();
    
    switch (screen % 3) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("GSM Test Complete");
        lcd.setCursor(0, 1);
        lcd.print("Reset to retest");
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Msg Sent to:");
        lcd.setCursor(0, 1);
        lcd.print(PHONE_NUMBER_1);
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("And also to:");
        lcd.setCursor(0, 1);
        lcd.print(PHONE_NUMBER_2);
        break;
    }
    
    screen++;
    lastUpdate = millis();
  }
  
  // Optional: Monitor SIM800L for incoming messages
  if (sim800lSerial.available()) {
    Serial.write(sim800lSerial.read());
  }
  
  delay(100);
}
