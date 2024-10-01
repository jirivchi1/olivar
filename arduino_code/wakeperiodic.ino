#include <RTClib.h>
#include <LowPower.h>

RTC_DS3231 rtc;

const int relePin = 7;    // Pin conectado al relé
const int horasActivacion[] = {18,19,20,21,22,23,00}; // Horas de activación
const int numActivaciones = sizeof(horasActivacion) / sizeof(horasActivacion[0]);

const int MAX_RTC_INIT_ATTEMPTS = 5;
bool rtcInitialized = false;

void setup() {
  Serial.begin(9600);
  
  // Intentar inicializar el RTC
  for (int attempts = 0; attempts < MAX_RTC_INIT_ATTEMPTS; attempts++) {
    if (rtc.begin()) {
      rtcInitialized = true;
      Serial.println("RTC inicializado correctamente.");
      break;
    }
    Serial.println("No se pudo inicializar el RTC. Reintentando...");
    delay(1000); // Esperar un segundo antes de reintentar
  }

  if (!rtcInitialized) {
    Serial.println("No se pudo inicializar el RTC después de múltiples intentos.");
    Serial.println("El programa continuará, pero la precisión del tiempo puede verse afectada.");
  }
  // Configurar la hora actual si se ha perdido la energía
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("RTC ajustado a la hora compilada.");
  }

  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, LOW);
}

void loop() {

  
  DateTime now = rtc.now();
  
  // Mostrar la hora actual
  Serial.print("Hora actual: ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  // Calcular el próximo intervalo de activación
  int nextHora = obtenerProximoIntervalo(now.hour());

  // Mostrar el próximo intervalo de activación
  Serial.print("Próximo intervalo de activación: ");
  Serial.print(nextHora);
  Serial.println(":00:00");

  // Calcular la diferencia de tiempo
  int diffHoras, diffMinutos, diffSegundos;
  calcularDiferencia(now, nextHora, diffHoras, diffMinutos, diffSegundos);

  // Mostrar la diferencia de tiempo
  Serial.print("Tiempo hasta el próximo intervalo: ");
  Serial.print(diffHoras);
  Serial.print(" horas, ");
  Serial.print(diffMinutos);
  Serial.print(" minutos, ");
  Serial.print(diffSegundos);
  Serial.println(" segundos.");

  // Convertir la diferencia a segundos para Deep Sleep
  unsigned long tiempoDormirSegundos = diffHoras * 3600UL + diffMinutos * 60UL + diffSegundos;
  Serial.print("Durmiendo por ");
  Serial.print(tiempoDormirSegundos);
  Serial.println(" segundos.");

  delay(3000);
  
  // Dormir hasta el próximo intervalo
  dormir(tiempoDormirSegundos);
  
  // Activar el relé si es el momento
  activarRele();

  // Esperar un segundo antes de la siguiente iteración
  delay(1000);
}

int obtenerProximoIntervalo(int horaActual) {
  for (int i = 0; i < numActivaciones; i++) {
    if (horasActivacion[i] > horaActual) {
      return horasActivacion[i];
    }
  }
  // Si ya pasó el último intervalo, regresar al primero del siguiente día
  return horasActivacion[0];
}

void calcularDiferencia(DateTime now, int nextHora, int &diffHoras, int &diffMinutos, int &diffSegundos) {
  int currentHora = now.hour();
  int currentMinuto = now.minute();
  int currentSegundo = now.second();

  // Convertir la hora actual y la próxima hora a segundos desde medianoche
  unsigned long nowTotalSegundos = currentHora * 3600UL + currentMinuto * 60UL + currentSegundo;
  unsigned long nextTotalSegundos = nextHora * 3600UL; // Los minutos y segundos del próximo intervalo son 0

  // Si el próximo intervalo ya pasó hoy, añadir 24 horas
  if (nextTotalSegundos <= nowTotalSegundos) {
    nextTotalSegundos += 24UL * 3600UL;
  }

  // Calcular la diferencia en segundos
  unsigned long diffSeg = nextTotalSegundos - nowTotalSegundos;

  // Descomponer la diferencia en horas, minutos y segundos
  diffHoras = diffSeg / 3600UL;
  diffMinutos = (diffSeg % 3600UL) / 60UL;
  diffSegundos = diffSeg % 60UL;
}

void dormir(unsigned long segundos) {
  Serial.print("Entrando en modo de bajo consumo por ");
  Serial.print(segundos);
  Serial.println(" segundos.");

  while (segundos > 0) {
    if (segundos >= 8) {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      segundos -= 8;
    } else if (segundos >= 2) { // Usar SLEEP_2S para optimizar
      LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
      segundos -= 2;
    } else {
      LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
      segundos -= 1;
    }
  }

  Serial.println("Despertado del modo de bajo consumo.");
}

void activarRele() {
  
  DateTime now = rtc.now();
  // Verificar si la hora actual coincide con alguna hora de activación y si los minutos y segundos son cero
  for (int i = 0; i < numActivaciones; i++) {
    if (now.hour() == horasActivacion[i] && now.minute() <= 8) {
      Serial.println("Activando relé...");
      digitalWrite(relePin, HIGH);
      delay(600000);  // Esperar 10 min.
      digitalWrite(relePin, LOW);
      Serial.println("Relé desactivado.");
      break;
    }
  }
}