#include <LowPower.h>

const int RELAY_PIN = 7;  // Pin del Arduino conectado al relé

// Configuración de tiempos
const unsigned long ACTIVATION_TIME = 120000;  // 2 minutos en milisegundos
const unsigned long EXTENDED_TIME = 240000;    // 4 minutos en milisegundos
const unsigned long SLEEP_TIME = 21600000;     // 6 horas en milisegundos
const unsigned long NIGHT_SLEEP_TIME = 43200000; // 12 horas en milisegundos (ajustable)

int photoCount = 0;
bool isDaytime = true;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Asegurarse de que el relé está apagado al inicio
}

void loop() {
  if (isDaytime) {
    takePhoto();
    photoCount++;
    
    if (photoCount >= 4) {
      switchToNight();
    }
  } else {
    sleepNight();
    switchToDay();
  }
}

void takePhoto() {
  digitalWrite(RELAY_PIN, HIGH);
  
  if (photoCount == 3) {
    // Última foto del ciclo, mantener encendido por 4 minutos
    delay(EXTENDED_TIME);
  } else {
    delay(ACTIVATION_TIME);
  }
  
  digitalWrite(RELAY_PIN, LOW);
  
  // Dormir entre fotos
  for (unsigned long i = 0; i < SLEEP_TIME; i += 8000) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void switchToNight() {
  isDaytime = false;
  photoCount = 0;
}

void switchToDay() {
  isDaytime = true;
}

void sleepNight() {
  // Dormir durante toda la noche
  for (unsigned long i = 0; i < NIGHT_SLEEP_TIME; i += 8000) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}