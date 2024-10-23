#include "RTClib.h"
#include "LowPower.h"
#include <SoftwareSerial.h>

RTC_DS3231 rtc;

const int wakeUpPin = 2; // Pin de interrupción para despertar
const int relayPin = 7; // Pin para controlar el relé
const int irSensorPin = 4; // Pin para el sensor infrarrojo
const int rxPin = 10; // Pin RX para comunicación serial con la Raspberry Pi
const int txPin = 11; // Pin TX para comunicación serial con la Raspberry Pi

unsigned long lastDebounceTime = 0; // Tiempo de la última interrupción
const unsigned long debounceDelay = 200; // Retardo para evitar rebotes (200 ms)

// Definir las horas y minutos de activación
const byte NUM_ALARMAS = 10;
byte horasActivacion[NUM_ALARMAS] = {7, 8, 9, 10, 11,13, 14, 15, 16, 17, 18};
byte minutosActivacion[NUM_ALARMAS] = {00, 00, 00, 00,00, 00, 00, 00, 00, 00, 00};

volatile int objectCount = 0; // Contador de objetos detectados
bool countingObjects = false; // Indicador de si el sensor infrarrojo está contando

SoftwareSerial raspberryPiSerial(rxPin, txPin); // Comunicación serial con la Raspberry Pi

void setup() {
  Serial.begin(9600); // Inicializar comunicación serial para monitorear
  raspberryPiSerial.begin(9600); // Inicializar comunicación serial con la Raspberry Pi

  if (!rtc.begin()) {
    Serial.println("No se pudo encontrar el RTC");
    // Intentar reiniciar el sistema después de un fallo
    delay(5000);
    asm volatile ("jmp 0"); // Reiniciar el Arduino
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(__DATE__, __TIME__)); // Establecer fecha y horario
  }

  // Deshabilitar y limpiar alarmas previas
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  rtc.writeSqwPinMode(DS3231_OFF); // Colocar el pin SQW en modo de interrupción de alarma

  pinMode(wakeUpPin, INPUT_PULLUP); // Configurar el pin de despertar como entrada con resistencia pull-up
  pinMode(relayPin, OUTPUT); // Configurar el pin del relé como salida
  pinMode(irSensorPin, INPUT); // Configurar el pin del sensor infrarrojo como entrada
  digitalWrite(relayPin, LOW); // Asegurarse de que el relé esté apagado inicialmente

  configurarSiguienteAlarma();
}

void loop() {
  delay(500); // Esperar antes de entrar en modo de bajo consumo para ver las salidas en serial
  // Entrar en modo de bajo consumo hasta que la alarma se dispare
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, LOW);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));

  // Determinar si es la alarma específica para realizar el conteo
  DateTime now = rtc.now();
  for (int i = 0; i < NUM_ALARMAS; i++) {
    if (now.hour() == horasActivacion[i] && now.minute() == minutosActivacion[i]) {
      if (now.hour() == 11) {
        // Cuando se despierte a las 7:00, activar el sensor infrarrojo y empezar a contar
        iniciarConteoInfrarrojo();
        activarRele2();
      } else {
        // Para todas las otras horas, activar el relé
        activarRele1();
      }
    }
  }

  // Limpiamos la alarma
  rtc.clearAlarm(1);

  // Configuramos la siguiente alarma
  configurarSiguienteAlarma();
}

void configurarSiguienteAlarma() {
  DateTime now = rtc.now();
  Serial.print(now.hour());
  Serial.print(":");
  Serial.println(now.minute());
  delay(500);

  for (int i = 0; i < NUM_ALARMAS; i++) {
    if (now.hour() < horasActivacion[i] || (now.hour() == horasActivacion[i] && now.minute() < minutosActivacion[i])) {
      rtc.setAlarm1(DateTime(0, 0, 0, horasActivacion[i], minutosActivacion[i], 0), DS3231_A1_Hour);
      Serial.print("Alarma establecida para: ");
      Serial.print(horasActivacion[i]);
      Serial.print(":");
      Serial.println(minutosActivacion[i]);
      return;
    }
  }
  // Si ya pasaron todas las alarmas del día, configurar para el primer horario del día siguiente
  rtc.setAlarm1(DateTime(0, 0, 0, horasActivacion[0], minutosActivacion[0], 0), DS3231_A1_Hour);
  Serial.print("Alarma establecida para mañana a las: ");
  Serial.print(horasActivacion[0]);
  Serial.print(":");
  Serial.println(minutosActivacion[0]);
}

void iniciarConteoInfrarrojo() {
  objectCount = 0;
  countingObjects = true;
  unsigned long startTime = millis();
  Serial.println("Iniciando conteo de objetos con el sensor infrarrojo...");

  while (millis() - startTime < 15000) { // Contar durante 1 hora (3600000 ms)
    if (digitalRead(irSensorPin) == LOW) { // Asumimos que LOW indica la detección de un objeto
      objectCount++;
      delay(100); // Pequeño retardo para evitar conteos múltiples del mismo objeto
    }
  }

  countingObjects = false;
  Serial.print("Conteo final de objetos: ");
  Serial.println(objectCount);
}

void activarRele1() {
  // Activar el relé durante 8 minutos
  digitalWrite(relayPin, HIGH);
  delay(480000); // Mantener el relé activado durante 8 minutos
  digitalWrite(relayPin, LOW); // Apagar el relé
}

void activarRele2() {
  // Activar el relé durante 4 minutos
  digitalWrite(relayPin, HIGH);
  delay(60000); // Mantener el relé activado durante 1 min.
  // Enviar el recuento de objetos a la Raspberry Pi
  enviarRecuentoAlaPi();
  delay(120000); // Mantener el relé activado durante 1 min.
  digitalWrite(relayPin, LOW); // Apagar el relé
}

void enviarRecuentoAlaPi() {
  Serial.print("Enviando recuento de objetos a la Raspberry Pi: ");
  Serial.println(objectCount);
  // Enviar el recuento a la Pi usando comunicación serial
  raspberryPiSerial.write(objectCount);
}

void wakeUp() {
  // Manejador de interrupción con debounce
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = currentTime;
    // No hacer nada más aquí, solo actualizar el tiempo de debounce
  }
}
