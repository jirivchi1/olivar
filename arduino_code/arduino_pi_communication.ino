#include <SoftwareSerial.h>

// Definir los pines para SoftwareSerial
const int rxPin = 10; // RX de SoftwareSerial (Recibe de Pi)
const int txPin = 11; // TX de SoftwareSerial (Envía a Pi)

SoftwareSerial mySerial(rxPin, txPin); // RX, TX

const int ldr = A0;

void setup(){
    pinMode(ldr, INPUT);             // Inicializar el sensor LDR como INPUT
    mySerial.begin(9600);            // Iniciar comunicación SoftwareSerial a 9600 bps
    Serial.begin(9600);              // Iniciar comunicación USB para depuración
    Serial.println("Arduino listo para comunicación bidireccional.");
}

void loop(){
    int rawValue = analogRead(ldr);  // Leer el valor analógico del LDR
    float voltage = (rawValue / 1023.0) * 5.0; // Calcular la tensión

    // Enviar la tensión a la Raspberry Pi
    mySerial.print("Voltage:");
    mySerial.println(voltage, 2);    // Enviar con 2 decimales

    // Enviar la tensión al Monitor Serial para depuración
    Serial.print("Voltage:");
    Serial.println(voltage, 2);        // Enviar con 2 decimales

    delay(10000); // Esperar 10 segundos antes de la siguiente lectura
}
