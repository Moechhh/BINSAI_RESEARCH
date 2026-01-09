// test/mq135_calibration.cpp
#include <Arduino.h>

const int MQ_PIN = 34;
float RL = 1.0; // Beban resistor 1k Ohm

void setup() {
  Serial.begin(115200);
  Serial.println("Calibrating MQ-135 in clean air...");
}

void loop() {
  float analogVal = analogRead(MQ_PIN);
  float v_out = (analogVal * 3.3) / 4095.0;
  float rs_air = ((3.3 * RL) / v_out) - RL;
  float r0 = rs_air / 3.6; // 3.6 adalah rasio standar MQ-135 di udara bersih
  
  Serial.print("V_Out: "); Serial.print(v_out);
  Serial.print(" | R0: "); Serial.println(r0);
  delay(1000);
}
