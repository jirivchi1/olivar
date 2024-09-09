const int ledPin = 13;
int currentHour;
 
void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);  // Inicia la comunicación serial
}
 
void loop() {
  if (Serial.available() > 0) {
    // Leer la hora enviada desde la Raspberry Pi
    currentHour = Serial.parseInt();
    
    // Condiciones para encender o apagar el LED
    if (currentHour >= 6 && currentHour < 18) {
      // Enciende el LED entre las 6:00 y las 18:00
      digitalWrite(ledPin, HIGH);
    } else {
      // Apaga el LED fuera de ese rango
      digitalWrite(ledPin, LOW);
    }
  }
}
 