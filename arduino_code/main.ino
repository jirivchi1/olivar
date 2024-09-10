#include "LowPower.h"
const int RELAY_PIN = 7;  // the Arduino pin, which connects to the IN pin of relay
const int ledpin = 13;
int currentHour;
void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ledpin, OUTPUT);
  Serial.begin(9600);  // Inicia la comunicación serial
}
 
void loop() {
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(ledpin,HIGH);
 
 
 if (Serial.available() > 0) {
    // Leer la hora enviada desde la Raspberry Pi
    currentHour = Serial.parseInt();
    Serial.println(currentHour);
    delay(5000);
    // Condiciones para encender o apagar el LED
    if (currentHour >= 6 && currentHour < 12) {
      delay(10000);
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(ledpin,LOW);
      for(int j=0;j<1;j++) LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
      Serial.println("sleeping 8s");
 
    } else {
      delay(1000);
      digitalWrite(RELAY_PIN, LOW);
      for(int j=0;j<2;j++) LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
      Serial.println("sleeping 16s");
    }
  }
 
  else{
    delay(5000);
    digitalWrite(RELAY_PIN, LOW);
    delay(5000);
    Serial.println("no hay conexion");
  }
 