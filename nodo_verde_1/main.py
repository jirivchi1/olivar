import os
import time
from datetime import datetime
import subprocess
import requests
import pytz
from dotenv import load_dotenv  # Importar la librería para manejar variables de entorno

# Cargar las variables desde el archivo .env
load_dotenv()

# Configuración del servidor desde variables de entorno
SERVER_USER = os.getenv("SERVER_USER")
SERVER_IP = os.getenv("SERVER_IP")
SERVER_DIR = os.getenv("SERVER_DIR")
LOCAL_DIRECTORY = os.getenv("LOCAL_DIRECTORY", "/home/pi/pruebas_campo/olivar/nodo_verde_1/fotos")
METRICS_DIRECTORY = os.getenv("METRICS_DIRECTORY", "/home/pi/pruebas_campo/olivar/metrics")

# Configuración de la URL de monitorización y otros datos sensibles
MONITORING_URL = os.getenv("MONITORING_URL")
DEVICE_ID = os.getenv("DEVICE_ID")
API_PASSWORD = os.getenv("API_PASSWORD")

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

def send_monitoring_data(filename):
    """Enviar minutos y segundos de la imagen para monitorización."""
    # Obtener minutos y segundos del nombre del archivo
    time_part = filename.split('_')[1]  # Extraemos el 'HHMMSS' del nombre
    minutes = time_part[2:4]
    seconds = time_part[4:6]
    
    # Obtener la fecha y hora actual en el formato adecuado
    zona_horaria = pytz.timezone('Europe/Madrid')
    now = datetime.now(zona_horaria)
    timestamp = now.strftime('%Y-%m-%d %H:%M:%S CEST%z')
    
    # Preparar datos para el envío
    data = {
        "name": "irivera",
        "password": API_PASSWORD,  # Usar la variable de entorno para el password
        "device_id": DEVICE_ID,
        "timestamp": timestamp,
        "segundos": int(minutes + seconds),  # Convertir minutos + segundos a formato correcto
        "infrarrojos": 5
    }

    # Enviar solicitud POST
    response = requests.post(MONITORING_URL, json=data, headers={"Content-Type": "application/json"})
    
    if response.status_code == 200:
        print("Datos de monitorización enviados correctamente.")
        log_action("Monitorización: datos enviados correctamente.")
    else:
        print(f"Error al enviar datos de monitorización. Código: {response.status_code}")
        log_action(f"Monitorización: error al enviar datos. Código: {response.status_code}")

def main():
    # Tomar la foto
    filepath, filename = take_photo()
    log_action(f"Photo {filename} taken.")

    # Subir todas las fotos al servidor
    upload_to_server()

    # Enviar datos de minutos y segundos para la monitorización
    send_monitoring_data(filename)

    # Eliminar las fotos del directorio local
    delete_photos()

    # Apagar el sistema
    shutdown_system()

if __name__ == "__main__":
    main()
