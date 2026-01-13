/*
================================================================================
   BINSAI RESEARCH SYSTEM - FIXED COMPILATION ERRORS
   Menggunakan library yang kompatibel dengan ESP32
================================================================================
*/

// ============ BLYNK CONFIGURATION ============
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE"
#define BLYNK_TEMPLATE_NAME "BINSAI"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH"
#define BLYNK_PRINT Serial

// ============ LIBRARIES YANG KOMPATIBEL ESP32 ============
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Pastikan library versi ESP32 compatible
#include <TinyGPSPlus.h>
#include <Preferences.h>

// ============ PIN DEFINITION ============
#define TRIG_PIN 5
#define ECHO_PIN 18
#define MQ135_PIN 34
#define SIM800_RX 16
#define SIM800_TX 17
#define SIM800_PWR 4
#define GPS_RX 13
#define GPS_TX 15
#define BUZZER_PIN 25

// ============ KONFIGURASI SISTEM ============
#define TINGGI_BAK 40.0
#define TH_KOSONG 35.0
#define TH_SETENGAH 50.0
#define TH_HAMPIR_PENUH 90.0

#define COEFF_A 0.002348
#define COEFF_B 2.856
#define PPM_BERSIH_MAX 199.0f
#define PPM_ANORGANIK_MAX 449.0f
#define PPM_ORGANIK_L1_MAX 800.0f
#define PPM_ORGANIK_L2_MIN 801.0f

#define CALIBRATION_TIME 60000
#define LCD_UPDATE_INTERVAL 2000
#define DISPLAY_ROTATE_INTERVAL 4000
#define GPS_CHECK_INTERVAL 10000
#define SMS_NOTIFICATION_TIME 5000

// Nomor Telepon
String phoneNumbers[] = {"+62_YOUR_NUMBER", "+62YOUR_NUMBER"};
const int JUMLAH_NOMOR = 2;

// WiFi Credentials (GANTI!)
char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PASS";
char auth[] = BLYNK_AUTH_TOKEN;

// ============ DEKLARASI OBJEK ============
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Pastikan address LCD benar
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
HardwareSerial SerialSIM(2);
Preferences prefs;

// ============ VARIABEL GLOBAL ============
float jarak = 0.0, persenKepenuhan = 0.0, ppmGas = 0.0;
float lintang = 0.0, bujur = 0.0;
String statusKapasitas = "KALIBRASI";
String jenisSampah = "KALIBRASI";
String rekomendasi = "MENYIAPKAN SISTEM";
int satelit = 0, prioritas = 0;
bool gpsValid = false, sim800Siap = false, blynkTerhubung = false;
bool kalibrasiSelesai = false, lcdActive = true;

// Variabel SMS
bool faseSMS = false, smsDalamProses = false, smsTerkirim = false;
String pesanSMS = "";
int smsCounter = 0, targetSMS = 0;
unsigned long waktuSMSTerakhir = 0, waktuNotifikasiMulai = 0;
unsigned long waktuSMSTimeout = 0;
#define SMS_COOLDOWN 300000

// Variabel Display
int modeDisplay = 0;
int counterRotasi = 0;
unsigned long waktuUpdate = 0;
unsigned long waktuDisplay = 0;
unsigned long waktuGPSCheck = 0;
bool tampilkanNotifikasi = false;
String notifikasiText = "";
unsigned long waktuNotifikasi = 0;

// GPS Variables
unsigned long lastGPSUpdate = 0;
bool gpsFirstFix = false;
int gpsAttemptCount = 0;

// Variabel untuk scrolling text (FIX: di luar fungsi)
int scrollPos = 0;
unsigned long lastScrollTime = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BINSAI SYSTEM - COMPATIBLE VERSION ===");
  
  // Inisialisasi dengan urutan yang benar
  initLCDWithRetry();
  initPins();
  
  // Tampilkan splash screen
  showSplashScreen();
  
  // Inisialisasi modul
  initSIM800LWithFeedback();
  initGPSWithFeedback();
  
  // Kalibrasi sensor
  kalibrasiMQ135WithProgress();
  
  // Koneksi jaringan
  connectWiFiWithStatus();
  connectBlynkWithStatus();
  
  // System ready
  showSystemReady();
}

// ============ FUNGSI MIN YANG AMAN ============
int safeMin(int a, int b) {
  return (a < b) ? a : b;
}

// ============ INISIALISASI LCD DENGAN RETRY ============
void initLCDWithRetry() {
  Serial.println("Initializing LCD...");
  Wire.begin(21, 22);
  delay(100);
  
  // Cek koneksi I2C
  Wire.beginTransmission(0x27);
  byte error = Wire.endTransmission();
  
  if (error != 0) {
    // Coba address alternatif
    Wire.beginTransmission(0x3F);
    error = Wire.endTransmission();
    if (error != 0) {
      Serial.println("ERROR: LCD tidak ditemukan!");
      lcdActive = false;
      return;
    } else {
      lcd = LiquidCrystal_I2C(0x3F, 16, 2);
      Serial.println("LCD found at 0x3F");
    }
  } else {
    Serial.println("LCD found at 0x27");
  }
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcdActive = true;
  
  lcd.print("LCD INIT OK");
  delay(1000);
}

void initPins() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SIM800_PWR, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(SIM800_PWR, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

// ============ SPLASH SCREEN ============
void showSplashScreen() {
  if (!lcdActive) return;
  
  lcd.clear();
  lcd.print("=== BINSAI ===");
  lcd.setCursor(0, 1);
  lcd.print("Research System");
  delay(2000);
}

// ============ INISIALISASI SIM800L ============
void initSIM800LWithFeedback() {
  if (lcdActive) {
    lcd.clear();
    lcd.print("INISIALISASI");
    lcd.setCursor(0, 1);
    lcd.print("MODUL GSM...");
  }
  
  Serial.println("Initializing SIM800L...");
  SerialSIM.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);
  delay(1000);
  
  // Power sequence
  digitalWrite(SIM800_PWR, HIGH);
  delay(1500);
  digitalWrite(SIM800_PWR, LOW);
  delay(3000);
  
  // Test AT Command
  String response = sendATCommand("AT", 2000);
  if (response.indexOf("OK") != -1) {
    sim800Siap = true;
    sendATCommand("ATE0", 1000);
    sendATCommand("AT+CMGF=1", 1000);
    
    if (lcdActive) {
      lcd.clear();
      lcd.print("SIM800L: SIAP");
      lcd.setCursor(0, 1);
      lcd.print("SMS Enabled");
    }
    Serial.println("SIM800L initialized");
    
    // Bunyi konfirmasi
    beep(2, 100);
    delay(2000);
    
  } else {
    sim800Siap = false;
    if (lcdActive) {
      lcd.clear();
      lcd.print("SIM800L: GAGAL");
      lcd.setCursor(0, 1);
      lcd.print("Cek koneksi");
    }
    Serial.println("SIM800L failed");
    delay(3000);
  }
}

String sendATCommand(String cmd, int timeout) {
  SerialSIM.println(cmd);
  Serial.print(">> ");
  Serial.println(cmd);
  
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < timeout) {
    while (SerialSIM.available()) {
      char c = SerialSIM.read();
      response += c;
    }
    
    if (response.indexOf("OK") != -1 || response.indexOf("ERROR") != -1) {
      break;
    }
    
    delay(10);
  }
  
  return response;
}

// ============ INISIALISASI GPS ============
void initGPSWithFeedback() {
  if (lcdActive) {
    lcd.clear();
    lcd.print("INISIALISASI");
    lcd.setCursor(0, 1);
    lcd.print("MODUL GPS...");
  }
  
  Serial.println("Initializing GPS...");
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  delay(1000);
  
  // Tunggu data GPS
  if (lcdActive) {
    lcd.clear();
    lcd.print("MENCARI SINYAL");
    lcd.setCursor(0, 1);
    lcd.print("GPS...");
  }
  
  unsigned long startTime = millis();
  bool gpsDataReceived = false;
  
  while (millis() - startTime < 10000 && !gpsDataReceived) {
    if (SerialGPS.available()) {
      char c = SerialGPS.read();
      if (c == '$') {
        gpsDataReceived = true;
      }
    }
    
    int elapsed = (millis() - startTime) / 1000;
    if (lcdActive && (elapsed % 2 == 0)) {
      lcd.setCursor(0, 1);
      lcd.print("Proses ");
      lcd.print(elapsed);
      lcd.print("s... ");
    }
    
    delay(100);
  }
  
  if (gpsDataReceived) {
    if (lcdActive) {
      lcd.clear();
      lcd.print("GPS: RESPOND");
      lcd.setCursor(0, 1);
      lcd.print("Menunggu fix...");
    }
    Serial.println("GPS responding");
  } else {
    if (lcdActive) {
      lcd.clear();
      lcd.print("GPS: NO DATA");
      lcd.setCursor(0, 1);
      lcd.print("Cek koneksi");
    }
    Serial.println("No GPS data");
  }
  delay(2000);
}

// ============ KALIBRASI MQ-135 ============
void kalibrasiMQ135WithProgress() {
  if (lcdActive) {
    lcd.clear();
    lcd.print("KALIBRASI MQ135");
    lcd.setCursor(0, 1);
    lcd.print("Udara bersih...");
  }
  
  unsigned long startTime = millis();
  
  while (millis() - startTime < CALIBRATION_TIME) {
    int remaining = (CALIBRATION_TIME - (millis() - startTime)) / 1000;
    
    if (lcdActive) {
      lcd.setCursor(0, 1);
      lcd.print("Sisa: ");
      lcd.print(remaining);
      lcd.print("s   ");
    }
    
    Serial.print(".");
    
    // Blink buzzer
    if ((millis() - startTime) % 2000 < 100) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(50);
      digitalWrite(BUZZER_PIN, LOW);
    }
    
    delay(1000);
  }
  
  kalibrasiSelesai = true;
  Serial.println("\nCalibration complete");
  
  if (lcdActive) {
    lcd.clear();
    lcd.print("KALIBRASI OK");
    lcd.setCursor(0, 1);
    lcd.print("Sistem siap!");
  }
  delay(1500);
}

// ============ KONEKSI JARINGAN ============
void connectWiFiWithStatus() {
  if (lcdActive) {
    lcd.clear();
    lcd.print("KONEKSI WiFi");
    lcd.setCursor(0, 1);
    lcd.print("Menghubungkan...");
  }
  
  WiFi.begin(ssid, pass);
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
    
    if (lcdActive && (attempts % 4 == 0)) {
      lcd.setCursor(0, 1);
      for (int i = 0; i < (attempts % 4); i++) {
        lcd.print(".");
      }
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    if (lcdActive) {
      lcd.clear();
      lcd.print("WiFi: TERHUBUNG");
      lcd.setCursor(0, 1);
      String ip = WiFi.localIP().toString();
      if (ip.length() > 16) {
        lcd.print(ip.substring(0, 13) + "...");
      } else {
        lcd.print(ip);
      }
    }
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
    beep(2, 100);
  } else {
    if (lcdActive) {
      lcd.clear();
      lcd.print("WiFi: GAGAL");
      lcd.setCursor(0, 1);
      lcd.print("Mode offline");
    }
    Serial.println("WiFi failed");
  }
  delay(2000);
}

void connectBlynkWithStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    blynkTerhubung = false;
    return;
  }
  
  if (lcdActive) {
    lcd.clear();
    lcd.print("KONEKSI BLYNK");
    lcd.setCursor(0, 1);
    lcd.print("Cloud server...");
  }
  
  Blynk.config(auth);
  if (Blynk.connect(5000)) {
    blynkTerhubung = true;
    Blynk.logEvent("system_start", "BINSAI Research System Online");
    
    if (lcdActive) {
      lcd.clear();
      lcd.print("Blynk: ONLINE");
      lcd.setCursor(0, 1);
      lcd.print("Cloud connected");
    }
    Serial.println("Blynk connected");
    beep(3, 80);
  } else {
    blynkTerhubung = false;
    if (lcdActive) {
      lcd.clear();
      lcd.print("Blynk: OFFLINE");
      lcd.setCursor(0, 1);
      lcd.print("Lokal mode");
    }
    Serial.println("Blynk failed");
  }
  delay(1500);
}

void beep(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < times - 1) delay(duration);
  }
}

// ============ SYSTEM READY ============
void showSystemReady() {
  if (!lcdActive) {
    Serial.println("=== SYSTEM READY ===");
    return;
  }
  
  lcd.clear();
  lcd.print("=== SISTEM SIAP ===");
  delay(1000);
  
  // Status modul
  lcd.clear();
  lcd.print("STATUS MODUL:");
  lcd.setCursor(0, 1);
  lcd.print("WiFi:");
  lcd.print(WiFi.status() == WL_CONNECTED ? "OK" : "NO");
  lcd.print(" GPS:");
  lcd.print(gpsValid ? "OK" : "NO");
  delay(2000);
  
  lcd.clear();
  lcd.print("SIM800:");
  lcd.print(sim800Siap ? "SIAP" : "ERR");
  lcd.print(" Blynk:");
  lcd.print(blynkTerhubung ? "OK" : "NO");
  delay(2000);
  
  // Startup complete
  lcd.clear();
  lcd.print("BINSAI RESEARCH");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM ACTIVE");
  
  // Victory beep
  beep(1, 300);
  delay(100);
  beep(1, 300);
  delay(100);
  beep(1, 500);
  
  delay(2000);
}

// ============ FUNGSI SENSOR ============
float bacaJarakHC_SR04() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long durasi = pulseIn(ECHO_PIN, HIGH, 30000);
  if (durasi == 0) return TINGGI_BAK;
  
  float jarak = durasi * 0.0343 / 2.0;
  if (jarak < 2.0 || jarak > 400.0) return TINGGI_BAK;
  
  return jarak;
}

float bacaMQ135() {
  long totalADC = 0;
  for (int i = 0; i < 5; i++) {
    totalADC += analogRead(MQ135_PIN);
    delay(2);
  }
  int adcValue = totalADC / 5;
  
  float ppm = COEFF_A * pow(adcValue, COEFF_B);
  
  if (adcValue < 60) {
    ppm = (ppm < PPM_BERSIH_MAX) ? ppm : PPM_BERSIH_MAX;
  } else if (adcValue >= 60 && adcValue <= 99) {
    if (ppm < 200.0f) ppm = 200.0f;
    if (ppm > PPM_ANORGANIK_MAX) ppm = PPM_ANORGANIK_MAX;
  } else if (adcValue >= 100 && adcValue <= 120) {
    if (ppm < 450.0f) ppm = 450.0f;
    if (ppm > PPM_ORGANIK_L1_MAX) ppm = PPM_ORGANIK_L1_MAX;
  } else {
    if (ppm < PPM_ORGANIK_L2_MIN) ppm = PPM_ORGANIK_L2_MIN;
  }
  
  return ppm;
}

void bacaGPS() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }
  
  if (gps.location.isUpdated() && gps.location.isValid()) {
    lintang = gps.location.lat();
    bujur = gps.location.lng();
    satelit = gps.satellites.value();
    
    if (satelit >= 3) {
      gpsValid = true;
      lastGPSUpdate = millis();
      
      if (!gpsFirstFix) {
        gpsFirstFix = true;
        showNotification("GPS FIX OK!", 3000);
        Serial.println("GPS First Fix!");
      }
    }
  }
  
  if (millis() - lastGPSUpdate > 10000) {
    gpsValid = false;
  }
}

// ============ KLASIFIKASI ============
void hitungDanKlasifikasi() {
  // Hitung persentase
  if (jarak >= TINGGI_BAK) {
    persenKepenuhan = 0.0;
  } else {
    persenKepenuhan = (1.0 - (jarak / TINGGI_BAK)) * 100.0;
  }
  
  if (persenKepenuhan < 0) persenKepenuhan = 0;
  if (persenKepenuhan > 100) persenKepenuhan = 100;
  
  // Status kapasitas
  if (persenKepenuhan <= TH_KOSONG) {
    statusKapasitas = "KOSONG";
  } else if (persenKepenuhan <= TH_SETENGAH) {
    statusKapasitas = "SETENGAH";
  } else if (persenKepenuhan <= TH_HAMPIR_PENUH) {
    statusKapasitas = "HAMPIR PENUH";
  } else {
    statusKapasitas = "PENUH!";
  }
  
  // Klasifikasi sampah
  if (ppmGas <= PPM_BERSIH_MAX) {
    jenisSampah = "NORMAL";
    prioritas = 0;
    rekomendasi = "Monitoring saja sudah cukup";
  } else if (ppmGas <= PPM_ANORGANIK_MAX) {
    jenisSampah = "ANORGANIK";
    prioritas = 1;
    rekomendasi = "Harap menyiapkan jadwal reguler";
  } else if (ppmGas <= PPM_ORGANIK_L1_MAX) {
    jenisSampah = "ORGANIK L1";
    prioritas = 2;
    rekomendasi = "Siapkan kantong khusus";
  } else {
    jenisSampah = "ORGANIK L2";
    prioritas = 3;
    rekomendasi = "SEGERA ANGKUT SEBELUM MEMBUSUK!";
  }
  
  // Cek trigger SMS
  if (persenKepenuhan > 90 && !faseSMS && sim800Siap && 
      (millis() - waktuSMSTerakhir > SMS_COOLDOWN)) {
    triggerSMSNotification();
  }
}

// ============ SISTEM NOTIFIKASI ============
void showNotification(String message, unsigned long duration) {
  if (!lcdActive) {
    Serial.print("NOTIF: ");
    Serial.println(message);
    return;
  }
  
  tampilkanNotifikasi = true;
  notifikasiText = message;
  waktuNotifikasi = millis();
  
  lcd.clear();
  lcd.print(">> NOTIFIKASI <<");
  lcd.setCursor(0, 1);
  
  // Tampilkan pesan
  if (message.length() > 16) {
    lcd.print(message.substring(0, 16));
  } else {
    lcd.print(message);
    // Tambahkan spasi
    for (int i = message.length(); i < 16; i++) {
      lcd.print(" ");
    }
  }
  
  // Bunyikan buzzer
  beep(2, 150);
  
  waktuNotifikasiMulai = millis();
}

// ============ SMS NOTIFICATION SYSTEM ============
void triggerSMSNotification() {
  faseSMS = true;
  smsDalamProses = true;
  waktuSMSTerakhir = millis();
  targetSMS = JUMLAH_NOMOR;
  smsCounter = 0;
  
  // Tampilkan notifikasi
  showNotification("SMS ALERT! >90%", 3000);
  delay(2000);
  
  // Buat pesan SMS
  pesanSMS = "Haloo. BINSAI_01 disini. Bak sampah telah mencapai >90%.\n";
  pesanSMS += "Anda perlu melakukan: " + rekomendasi + "\n\n";
  pesanSMS += "Detail Status:\n";
  pesanSMS += "Kapasitas: " + String((int)persenKepenuhan) + "%\n";
  pesanSMS += "Status: " + statusKapasitas + "\n";
  pesanSMS += "Jenis Sampah: " + jenisSampah + "\n";
  pesanSMS += "Prioritas: " + String(prioritas) + "\n";
  pesanSMS += "Gas Level: " + String((int)ppmGas) + " ppm\n\n";
  
  if (gpsValid) {
    pesanSMS += "Silakan menuju lokasi berikut:\n";
    pesanSMS += "Lokasi: https://maps.google.com/?q=";
    pesanSMS += String(lintang, 6) + "," + String(bujur, 6);
  } else {
    pesanSMS += "Lokasi: GPS sedang mencari sinyal";
  }
  
  // Mulai proses pengiriman
  waktuSMSTimeout = millis();
}

void processSMS() {
  if (!smsDalamProses) return;
  
  unsigned long currentTime = millis();
  
  // Timeout protection
  if (currentTime - waktuSMSTimeout > 30000) {
    showNotification("SMS TIMEOUT!", 3000);
    smsDalamProses = false;
    faseSMS = false;
    return;
  }
  
  // Kirim SMS ke setiap nomor
  while (smsCounter < targetSMS) {
    // Tampilkan status di LCD
    if (lcdActive) {
      lcd.clear();
      lcd.print("KIRIM SMS ke:");
      lcd.setCursor(0, 1);
      lcd.print("Nomor ");
      lcd.print(smsCounter + 1);
      lcd.print("/");
      lcd.print(targetSMS);
    }
    
    Serial.print("Mengirim SMS ke: ");
    Serial.println(phoneNumbers[smsCounter]);
    
    // Kirim SMS
    if (kirimSMS(phoneNumbers[smsCounter], pesanSMS)) {
      smsCounter++;
      
      // Tampilkan konfirmasi
      if (lcdActive) {
        lcd.clear();
        lcd.print("SMS ");
        lcd.print(smsCounter);
        lcd.print(" TERKIRIM!");
        delay(2000);
      }
      
      beep(1, 200);
      delay(2000);
      
    } else {
      // Gagal
      showNotification("SMS GAGAL! Coba lagi", 2000);
      delay(2000);
      smsCounter++;
    }
    
    delay(1000);
  }
  
  // Semua SMS terkirim
  if (smsCounter >= targetSMS) {
    smsDalamProses = false;
    faseSMS = false;
    smsTerkirim = true;
    
    // Tampilkan notifikasi sukses
    showNotification("SMS SEMUA TERKIRIM!", 5000);
    Serial.println("All SMS sent");
    
    // Update Blynk
    if (blynkTerhubung) {
      Blynk.logEvent("sms_sent", String(targetSMS) + " SMS sent for >90% capacity");
    }
    
    delay(3000);
  }
}

bool kirimSMS(String nomor, String pesan) {
  // Kirim AT Command untuk SMS
  String response = sendATCommand("AT+CMGS=\"" + nomor + "\"", 5000);
  
  if (response.indexOf(">") != -1) {
    // Kirim isi pesan
    SerialSIM.print(pesan);
    delay(100);
    SerialSIM.write(26);  // Ctrl+Z
    
    // Tunggu konfirmasi
    response = "";
    unsigned long start = millis();
    
    while (millis() - start < 10000) {
      while (SerialSIM.available()) {
        char c = SerialSIM.read();
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
  }
  
  return false;
}

// ============ UPDATE DISPLAY DENGAN NOTIFIKASI (FIXED) ============
void updateDisplayWithNotifications() {
  if (!lcdActive) return;
  
  unsigned long currentTime = millis();
  
  // Priority 1: Tampilkan notifikasi jika ada
  if (tampilkanNotifikasi && (currentTime - waktuNotifikasi < SMS_NOTIFICATION_TIME)) {
    // Notifikasi masih aktif
    return;
  } else if (tampilkanNotifikasi) {
    // Notifikasi selesai
    tampilkanNotifikasi = false;
    lcd.clear();
  }
  
  // Priority 2: Tampilkan status SMS jika sedang proses
  if (smsDalamProses) {
    lcd.clear();
    lcd.print("PROSES SMS:");
    lcd.setCursor(0, 1);
    lcd.print(smsCounter);
    lcd.print("/");
    lcd.print(targetSMS);
    lcd.print(" sent");
    return;
  }
  
  // Priority 3: Rotasi display normal
  if (currentTime - waktuDisplay > DISPLAY_ROTATE_INTERVAL) {
    waktuDisplay = currentTime;
    lcd.clear();
    
    // Rotasi 4 mode (FIXED: tanpa error min())
    switch (modeDisplay) {
      case 0: // Kapasitas
        lcd.print("KAPASITAS BAK");
        lcd.setCursor(0, 1);
        lcd.print((int)persenKepenuhan);
        lcd.print("% - ");
        lcd.print(statusKapasitas);
        break;
        
      case 1: // Jenis Sampah
        lcd.print("JENIS SAMPAH");
        lcd.setCursor(0, 1);
        lcd.print(jenisSampah);
        lcd.print(" (P");
        lcd.print(prioritas);
        lcd.print(")");
        break;
        
      case 2: // Rekomendasi
        lcd.print("REKOMENDASI");
        lcd.setCursor(0, 1);
        if (rekomendasi.length() > 16) {
          // Scroll text dengan cara yang aman (FIXED)
          unsigned long currentScrollTime = millis();
          if (currentScrollTime - lastScrollTime > 500) { // Scroll setiap 500ms
            lastScrollTime = currentScrollTime;
            
            // Gunakan fungsi safeMin yang sudah dibuat
            int endPos = scrollPos + 16;
            int textLength = rekomendasi.length();
            
            if (endPos > textLength) {
              endPos = textLength;
            }
            
            String displayText = rekomendasi.substring(scrollPos, endPos);
            lcd.print(displayText);
            
            // Update scroll position
            scrollPos++;
            if (scrollPos > textLength + 5) {
              scrollPos = 0;
            }
          }
        } else {
          lcd.print(rekomendasi);
        }
        break;
        
      case 3: // GPS & System Status
        lcd.print("SISTEM STATUS");
        lcd.setCursor(0, 1);
        lcd.print("GPS:");
        lcd.print(gpsValid ? "OK" : "NO");
        lcd.print(" SIM:");
        lcd.print(sim800Siap ? "ON" : "OFF");
        break;
    }
    
    modeDisplay = (modeDisplay + 1) % 4;
  }
}

// ============ GPS STATUS CHECK ============
void checkGPSStatus() {
  if (millis() - waktuGPSCheck > GPS_CHECK_INTERVAL) {
    waktuGPSCheck = millis();
    
    // Update GPS data
    bacaGPS();
    
    // Tampilkan status GPS secara berkala
    if (gpsValid && (millis() % 30000 < 1000)) {
      if (lcdActive && !tampilkanNotifikasi && !smsDalamProses) {
        lcd.clear();
        lcd.print("GPS STATUS:");
        lcd.setCursor(0, 1);
        lcd.print(satelit);
        lcd.print(" sat - OK");
        delay(2000);
      }
    } else if (!gpsValid && gpsAttemptCount < 3) {
      gpsAttemptCount++;
      
      if (lcdActive && !tampilkanNotifikasi && !smsDalamProses) {
        lcd.clear();
        lcd.print("GPS: MENCARI...");
        lcd.setCursor(0, 1);
        lcd.print("Attempt ");
        lcd.print(gpsAttemptCount);
        lcd.print("/3");
        delay(1000);
      }
    }
    
    // Reset attempt counter jika GPS fix
    if (gpsValid) {
      gpsAttemptCount = 0;
    }
  }
}

// ============ UPDATE BLYNK DATA ============
void updateBlynkData() {
  if (!blynkTerhubung) return;
  
  try {
    Blynk.virtualWrite(V0, (int)persenKepenuhan);
    Blynk.virtualWrite(V1, (persenKepenuhan >= 90) ? 255 : 0);
    Blynk.virtualWrite(V2, (persenKepenuhan >= 70 && persenKepenuhan < 90) ? 255 : 0);
    Blynk.virtualWrite(V3, (persenKepenuhan >= 40 && persenKepenuhan < 70) ? 255 : 0);
    Blynk.virtualWrite(V4, (persenKepenuhan < 40) ? 255 : 0);
    Blynk.virtualWrite(V5, jarak);
    Blynk.virtualWrite(V6, statusKapasitas);
    Blynk.virtualWrite(V10, (int)ppmGas);
    Blynk.virtualWrite(V11, prioritas);
    Blynk.virtualWrite(V12, jenisSampah);
    Blynk.virtualWrite(V13, rekomendasi);
    
    if (gpsValid) {
      Blynk.virtualWrite(V20, lintang);
      Blynk.virtualWrite(V21, bujur);
    }
    
    // Update warna
    String color = "#00FF00";
    if (statusKapasitas == "PENUH!") color = "#FF0000";
    else if (statusKapasitas == "HAMPIR PENUH") color = "#FF9900";
    else if (statusKapasitas == "SETENGAH") color = "#FFFF00";
    
    Blynk.setProperty(V6, "color", color);
    
  } catch (...) {
    Serial.println("Error updating Blynk");
  }
}

// ============ LOOP UTAMA ============
void loop() {
  unsigned long currentLoopTime = millis();
  
  // 1. Run Blynk
  if (blynkTerhubung) {
    Blynk.run();
  }
  
  // 2. Update sensor setiap 2 detik
  if (currentLoopTime - waktuUpdate > 2000) {
    waktuUpdate = currentLoopTime;
    
    // Baca semua sensor
    jarak = bacaJarakHC_SR04();
    ppmGas = bacaMQ135();
    bacaGPS();
    
    // Proses data
    hitungDanKlasifikasi();
    
    // Update Blynk
    updateBlynkData();
    
    // Debug ke Serial Monitor
    Serial.print("[SENSOR] Jarak: ");
    Serial.print(jarak);
    Serial.print("cm | Kap: ");
    Serial.print(persenKepenuhan);
    Serial.print("% | PPM: ");
    Serial.print(ppmGas);
    Serial.print(" | GPS: ");
    Serial.print(gpsValid ? "OK" : "NO");
    Serial.print(" | SMS: ");
    Serial.println(sim800Siap ? "Ready" : "Off");
  }
  
  // 3. Check GPS Status
  checkGPSStatus();
  
  // 4. Proses SMS jika triggered
  if (faseSMS) {
    processSMS();
  }
  
  // 5. Update Display
  updateDisplayWithNotifications();
  
  // 6. Delay kecil
  delay(50);
}
