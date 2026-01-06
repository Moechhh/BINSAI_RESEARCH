// ============================================================
// BINSAI V2.0 - SMART WASTE MANAGEMENT SYSTEM
// ============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32_SSL.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <Preferences.h>

// Include file secrets (aman)
#include "secrets.h"

// Konfigurasi dari secrets.h
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;
String phoneNumbers[] = {PHONE_NUMBER_1, PHONE_NUMBER_2};
const int phoneCount = 2;
const float TINGGI_BAK = BIN_HEIGHT_CM;
const float R0 = MQ135_RO_CALIBRATED;

// ... (sisa kode sama seperti sebelumnya)