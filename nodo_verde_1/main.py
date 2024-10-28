import os
import time
from datetime import datetime
import subprocess
import requests
import pytz
import serial
from dotenv import load_dotenv  # Importar la librería para manejar variables de entorno

# Cargar las variables desde el archivo .env
load_dotenv()

# Configuración del servidor desde variables de entorno
SERVER_USER = os.getenv("SERVER_USER")
SERVER_IP = os.getenv("SERVER_IP")
SERVER_DIR = os.getenv("SERVER_DIR")
LOCAL_DIRECTORY = os.getenv("LOCAL_DIRECTORY", "/home/pi/pruebas_campo/olivar/nodo_verde_1/fotos")
METRICS_DIRECTORY = os.getenv("METRICS_DIRECTORY", "/home/pi/pruebas_campo/olivar/metrics")
INFRARED_FILE = os.getenv("INFRARED_FILE", "/home/pi/pruebas_campo/olivar/nodo_verde_1/infrared_count.txt")

# Configuración de la URL de monitorización y otros datos sensibles
MONITORING_URL = os.getenv("MONITORING_URL")
DEVICE_ID = os.getenv("DEVICE_ID")
API_PASSWORD = os.getenv("API_PASSWORD")

def read_arduino_data():
    """
    Lee datos del Arduino durante 10 segundos.
    Retorna el valor leído o None si no se recibe nada.
    """
    try:
        # Configurar el puerto serie
        ser = serial.Serial('/dev/serial0', 9600, timeout=1)
        start_time = time.time()

        # Esperar hasta 10 segundos por datos
        while (time.time() - start_time) < 10:
            if ser.in_waiting > 0:
                data = ser.readline().decode('utf-8').strip()
                try:
                    valor = int(data)
                    log_action(f"Dato recibido del Arduino: {valor}")
                    ser.close()
                    # Guardar el valor en el archivo .txt (sobrescribir para mantener solo un dato)
                    with open(INFRARED_FILE, "w") as f:
                        f.write(str(valor))
                    return valor
                except ValueError:
                    continue

        ser.close()
        log_action("No se recibieron datos del Arduino en 10 segundos")
        return None

    except serial.SerialException as e:
        log_action(f"Error al conectar con Arduino: {str(e)}")
        return None

def take_photo():
    os.makedirs(LOCAL_DIRECTORY, exist_ok=True)
    filename = datetime.now().strftime("%Y%m%d_%H%M%S") + "_verde_RP07" + ".jpg"
    filepath = f"{LOCAL_DIRECTORY}/{filename}"
    subprocess.run(["fswebcam", "-r", "1280x720", "--no-banner", filepath])
    return filepath, filename

def upload_to_server():
    # Subir todas las fotos en el directorio LOCAL_DIRECTORY al servidor
    for filename in os.listdir(LOCAL_DIRECTORY):
        filepath = os.path.join(LOCAL_DIRECTORY, filename)
        if filename.endswith(".jpg"):
            subprocess.run(["scp", filepath, f"{SERVER_USER}@{SERVER_IP}:{SERVER_DIR}"])
            print(f"Image {filename} uploaded to the server.")
    print("All images uploaded.")

def delete_photos():
    # Eliminar todas las fotos en el directorio LOCAL_DIRECTORY
    for filename in os.listdir(LOCAL_DIRECTORY):
        filepath = os.path.join(LOCAL_DIRECTORY, filename)
        if filename.endswith(".jpg"):
            os.remove(filepath)
            print(f"Image {filename} deleted.")
    print("All images deleted.")

def log_action(message):
    with open(f"{METRICS_DIRECTORY}/log_verde.txt", "a") as log_file:
        log_file.write(f"{datetime.now()}: {message}\n")

def shutdown_system():
    subprocess.run(["sudo", "shutdown", "-h", "now"])

def send_monitoring_data(filename, infrared_count=None):
    """
    Enviar minutos y segundos de la imagen para monitorización.
    Ahora también envía el conteo infrarrojo si está disponible.
    """
    # Obtener minutos y segundos del nombre del archivo
    time_part = filename.split('_')[1]  # Extraemos el 'HHMMSS' del nombre
    minutes = time_part[2:4]
    seconds = time_part[4:6]

    # Obtener la fecha y hora actual en el formato adecuado
    zona_horaria = pytz.timezone('Europe/Madrid')
    now = datetime.now(zona_horaria)
    timestamp = now.strftime('%Y-%m-%d %H:%M:%S CEST%z')

    # Evaluar el valor de infrarrojo antes de crear el diccionario
    valor_infrarrojo = infrared_count if infrared_count is not None else 1

    # Preparar datos para el envío
    data = {
        "name": "irivera",
        "password": API_PASSWORD,
        "device_id": DEVICE_ID,
        "timestamp": timestamp,
        "segundos": int(minutes + seconds),
        "infrarrojo": valor_infrarrojo
    }

    try:
        # Enviar solicitud POST
        response = requests.post(MONITORING_URL, json=data, headers={"Content-Type": "application/json"})

        if response.status_code == 200:
            print("Datos de monitorización enviados correctamente.")
            log_action("Monitorización: datos enviados correctamente.")
            # Limpiar el archivo de infrarrojo después de enviar los datos
            open(INFRARED_FILE, "w").close()
        else:
            print(f"Error al enviar datos de monitorización. Código: {response.status_code}")
            log_action(f"Monitorización: error al enviar datos. Código: {response.status_code}")

    except requests.exceptions.ConnectionError as e:
        # Manejar el error de conexión y guardarlo en el log
        print(f"Error de conexión: {e}")
        log_action(f"Error de conexión al enviar datos: {e}")

def check_infrared_file():
    """
    Verificar si hay un valor almacenado en el archivo .txt y devolverlo.
    Si el archivo está vacío, devolver None.
    """
    if os.path.exists(INFRARED_FILE):
        with open(INFRARED_FILE, "r") as f:
            data = f.read().strip()
            if data:
                try:
                    return int(data)
                except ValueError:
                    log_action("Error: Valor inválido en el archivo de infrarrojo.")
                    return None
    return None

def main():
    # Verificar si hay un dato pendiente de infrarrojo
    infrared_count = check_infrared_file()

    # Intentar leer datos del Arduino si no hay un dato pendiente
    if infrared_count is None:
        infrared_count = read_arduino_data()

    # Tomar la foto
    filepath, filename = take_photo()
    log_action(f"Photo {filename} taken.")

    # Subir todas las fotos al servidor
    upload_to_server()

    # Enviar datos de monitorización incluyendo el conteo infrarrojo
    send_monitoring_data(filename, infrared_count)

    # Eliminar las fotos del directorio local
    delete_photos()

    # Apagar el sistema
    # shutdown_system()

if __name__ == "__main__":
    main()
