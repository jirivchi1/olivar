#include "RTClib.h"
#include "LowPower.h"

RTC_DS3231 rtc;

const int wakeUpPin = 2; // Pin de interrupción para despertar
const int relayPin = 7; // Pin para controlar el relé
unsigned long lastDebounceTime = 0; // Tiempo de la última interrupción
const unsigned long debounceDelay = 200; // Retardo para evitar rebotes (200 ms)

// Definir las horas y minutos de activación
const byte NUM_ALARMAS = 10;
byte horasActivacion[NUM_ALARMAS] = {7,8,9,10,13,14,15,16,17,18};
byte minutosActivacion[NUM_ALARMAS] = {00,00,00,00,00,00,00,00,00,00};

void setup() {
  Serial.begin(9600); // Inicializar comunicación serial para monitorear

  if (!rtc.begin()) {
    Serial.println("No se pudo encontrar el RTC");
    // Intentar reiniciar el sistema después de un fallo
    delay(5000);
    asm volatile ("jmp 0"); // Reiniciar el Arduino
  }

  if(rtc.lostPower()){
  rtc.adjust(DateTime(__DATE__, __TIME__));	// funcion que permite establecer fecha y horario
 }

  // Deshabilitar y limpiar alarmas previas
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  rtc.writeSqwPinMode(DS3231_OFF); // Colocar el pin SQW en modo de interrupción de alarma

  pinMode(wakeUpPin, INPUT_PULLUP); // Configurar el pin de despertar como entrada con resistencia pull-up
  pinMode(relayPin, OUTPUT); // Configurar el pin del relé como salida
  digitalWrite(relayPin, LOW); // Asegurarse de que el relé esté apagado inicialmente

  configurarSiguienteAlarma();
}



void loop() {
  delay(500); // Esperar antes de entrar en modo de bajo consumo para ver las salidas en serial
  // Entrar en modo de bajo consumo hasta que la alarma se dispare
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, LOW);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));

  activarRele();

    
  // Limpiamos la alarma
  rtc.clearAlarm(1);

  // Configuramos la siguiente alarma
  configurarSiguienteAlarma();
  }


void configurarSiguienteAlarma() {
  DateTime now = rtc.now();
  Serial.print(now.hour());
  Serial.print(":");				// caracter dos puntos como separador
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
  // Si ya pasaron todas las alarmas del día, configurar para el primer horario del día siguiente de manera sencilla
  rtc.setAlarm1(DateTime(0, 0, 0, horasActivacion[0], minutosActivacion[0], 0), DS3231_A1_Hour);
  Serial.print("Alarma establecida para mañana a las: ");
  Serial.print(horasActivacion[0]);
  Serial.print(":");
  Serial.println(minutosActivacion[0]);
}

void activarRele() {
  // Activar el relé durante 1 minuto
  digitalWrite(relayPin, HIGH);
  delay(540000); // Mantener el relé activado durante 8 minuto
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
