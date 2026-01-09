#include <Arduino.h>
#include <TinyGPS++.h>

// GPS Hardware Serial 1
HardwareSerial ss(1);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  ss.begin(9600, SERIAL_8N1, 13, 15); // RX, TX
  pinMode(5, OUTPUT); // TRIG
  pinMode(18, INPUT); // ECHO
}

void loop() {
  // Check HC-SR04
  digitalWrite(5, LOW); delayMicroseconds(2);
  digitalWrite(5, HIGH); delayMicroseconds(10);
  digitalWrite(5, LOW);
  long duration = pulseIn(18, HIGH);
  Serial.print("Distance: "); Serial.print(duration * 0.034 / 2); Serial.println(" cm");

  // Check GPS Satellites
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      Serial.print("Lat: "); Serial.println(gps.location.lat(), 6);
    }
  }
  delay(1000);
}
