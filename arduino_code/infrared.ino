const int led = 13;
const int sensor = 2;
int valor;
int valorAnterior = HIGH; // Estado anterior del sensor
int contador = 0;  // Variable para contar las detecciones

void setup() {
  pinMode(led, OUTPUT);
  pinMode(sensor, INPUT);
  Serial.begin(9600);  // Inicia la comunicación serial
}

void loop() {
  valor = digitalRead(sensor);

  // Detecta la transición de HIGH a LOW
  if (valor == LOW && valorAnterior == HIGH) {
    contador++;  // Incrementa el contador solo cuando hay una transición de HIGH a LOW
    Serial.println(contador);  // Imprime el contador por el puerto serial
  }

  // Guarda el valor actual para la siguiente iteración
  valorAnterior = valor;

  digitalWrite(led, !valor);
  delay(100);
}
