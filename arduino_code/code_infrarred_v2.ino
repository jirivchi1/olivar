#include "RTClib.h"
#include "LowPower.h"
#include <SoftwareSerial.h>

RTC_DS3231 rtc;

const int wakeUpPin = 2; // Pin de interrupción para despertar
const int relayPin = 7; // Pin para controlar el relé
const int rxPin = 10; // Pin RX para comunicación serial con la Raspberry Pi
const int txPin = 11; // Pin TX para comunicación serial con la Raspberry Pi

const int irSensorPin1 = 4; // Pin para el primer sensor infrarrojo
const int irSensorPin2 = 5; // Pin para el segundo sensor infrarrojo
volatile int objectCount1 = 0; // Contador de objetos detectados por el primer sensor
volatile int objectCount2 = 0; // Contador de objetos detectados por el segundo sensor



unsigned long lastDebounceTime = 0; // Tiempo de la última interrupción
const unsigned long debounceDelay = 200; // Retardo para evitar rebotes (200 ms)

// Definir las horas y minutos de activación
const byte NUM_ALARMAS = 10;
byte horasActivacion[NUM_ALARMAS] = {7, 8, 9, 10, 11,13, 14, 15, 16, 17, 18};
byte minutosActivacion[NUM_ALARMAS] = {00, 00, 00, 00,00, 00, 00, 00, 00, 00, 00};

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

void iniciarConteoInfrarrojo() {
  objectCount1 = 0; // Reiniciar el contador del primer sensor
  objectCount2 = 0; // Reiniciar el contador del segundo sensor
  int valorAnterior1 = HIGH;  // Estado anterior del primer sensor
  int valorAnterior2 = HIGH;  // Estado anterior del segundo sensor
  unsigned long tiempoInicio = millis();  // Guardar el tiempo de inicio
  Serial.println("Iniciando conteo de objetos con ambos sensores infrarrojos...");

  // Realizar el conteo durante 15 segundos
  while (millis() - tiempoInicio < 15000) {
    int valor1 = digitalRead(irSensorPin1);
    int valor2 = digitalRead(irSensorPin2);

    // Detecta la transición de HIGH a LOW para el primer sensor
    if (valor1 == LOW && valorAnterior1 == HIGH) {
      objectCount1++;  // Incrementa el contador del primer sensor
      Serial.print("Detecciones Sensor 1: ");
      Serial.println(objectCount1);
    }

    // Detecta la transición de HIGH a LOW para el segundo sensor
    if (valor2 == LOW && valorAnterior2 == HIGH) {
      objectCount2++;  // Incrementa el contador del segundo sensor
      Serial.print("Detecciones Sensor 2: ");
      Serial.println(objectCount2);
    }

    // Guarda los valores actuales para la siguiente iteración
    valorAnterior1 = valor1;
    valorAnterior2 = valor2;
    delay(100);
  }
  
  int conteoTotal = objectCount1 + objectCount2; // Suma de ambos contadores
  Serial.print("Conteo final de objetos total: ");
  Serial.println(conteoTotal);

  // Activar el relé y enviar datos
  digitalWrite(relayPin, HIGH);
  delay(60000); // Mantener el relé activado durante 1 min.
  
  // Enviar el recuento a la Pi usando comunicación serial
  Serial.print("Enviando recuento de objetos a la Raspberry Pi: ");
  Serial.println(objectCount);
  raspberryPiSerial.println(objectCount);
  
  delay(120000); // Mantener el relé activado durante 2 min adicionales
  digitalWrite(relayPin, LOW); // Apagar el relé
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

void activarRele1() {
  // Activar el relé durante 8 minutos
  digitalWrite(relayPin, HIGH);
  delay(480000); // Mantener el relé activado durante 8 minutos
  digitalWrite(relayPin, LOW); // Apagar el relé
}

void wakeUp() {
  // Manejador de interrupción con debounce
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = currentTime;
    // No hacer nada más aquí, solo actualizar el tiempo de debounce
  }
}
