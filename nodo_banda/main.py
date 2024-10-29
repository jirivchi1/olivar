import os
import time
from datetime import datetime
import subprocess
import requests
import pytz
from sht20 import SHT20
from dotenv import load_dotenv
import serial

# Cargar las variables desde el archivo .env
load_dotenv()

# Configuración del servidor desde variables de entorno
SERVER_USER = os.getenv("SERVER_USER")
SERVER_IP = os.getenv("SERVER_IP")
SERVER_DIR = os.getenv("SERVER_DIR")
LOCAL_DIRECTORY = os.getenv("LOCAL_DIRECTORY", "/home/pi/pruebas_campo/olivar/nodo_banda/fotos")
METRICS_DIRECTORY = os.getenv("METRICS_DIRECTORY", "/home/pi/pruebas_campo/olivar/metrics")
SENSOR_DATA_FILE = os.getenv("SENSOR_DATA_FILE", "/home/pi/pruebas_campo/olivar/nodo_banda/datos_sensor.txt")

# Configuración de la URL de monitorización y otros datos sensibles
MONITORING_URL = os.getenv("MONITORING_URL")
DEVICE_ID = os.getenv("DEVICE_ID")
API_PASSWORD = os.getenv("API_PASSWORD")

sht = SHT20(1, resolution=SHT20.TEMP_RES_14bit)

# Configuración del puerto serial
ser = serial.Serial('/dev/serial0', 9600, timeout=1)
time.sleep(2)

def take_photo():
    os.makedirs(LOCAL_DIRECTORY, exist_ok=True)
    filename = datetime.now().strftime("%Y%m%d_%H%M%S") + "_banda_RP06" + ".jpg"
    filepath = f"{LOCAL_DIRECTORY}/{filename}"
    subprocess.run(["fswebcam", "-r", "1280x720", "--no-banner", filepath])
    return filepath, filename

def read_sensor_data():
    data = sht.read_all()
    temp = round(data[0], 2)
    humid = round(data[1], 2)
    return temp, humid

def read_battery_data():
    """Captura una sola vez el dato de voltaje desde el Arduino."""
    bateriaArduino, bateriaPi = None, None
    try:
        for _ in range(10):  # Esperar hasta 10 lecturas, máximo 10 segundos
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                if "Voltage A1" in line:
                    bateriaArduino = float(line.split(":")[1].strip().replace("V", ""))
                elif "Voltage A0" in line:
                    bateriaPi = float(line.split(":")[1].strip().replace("V", ""))
                if bateriaArduino and bateriaPi:
                    break
            time.sleep(1)
    except KeyboardInterrupt:
        print("Terminando la lectura")
    finally:
        ser.close()
    return bateriaArduino, bateriaPi

def log_sensor_data(temp, humid):
    with open(SENSOR_DATA_FILE, "a") as file:
        file.write(f"{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}, T: {temp}, H: {humid}\n")

def upload_to_server():
    for filename in os.listdir(LOCAL_DIRECTORY):
        filepath = os.path.join(LOCAL_DIRECTORY, filename)
        if filename.endswith(".jpg"):
            subprocess.run(["scp", filepath, f"{SERVER_USER}@{SERVER_IP}:{SERVER_DIR}"])
            print(f"Image {filename} uploaded to the server.")
    subprocess.run(["scp", SENSOR_DATA_FILE, f"{SERVER_USER}@{SERVER_IP}:{SERVER_DIR}/datos_sensor.txt"])
    print("Sensor data uploaded to the server.")
    print("All images and sensor data uploaded.")

def delete_photos():
    for filename in os.listdir(LOCAL_DIRECTORY):
        filepath = os.path.join(LOCAL_DIRECTORY, filename)
        if filename.endswith(".jpg"):
            os.remove(filepath)
            print(f"Image {filename} deleted.")
    print("All images deleted.")

def log_action(message):
    with open(f"{METRICS_DIRECTORY}/log_banda.txt", "a") as log_file:
        log_file.write(f"{datetime.now()}: {message}\n")

def shutdown_system():
    subprocess.run(["sudo", "shutdown", "-h", "now"])

def send_monitoring_data(filename, temp, humid, bateriaArduino, bateriaPi):
    time_part = filename.split('_')[1]
    minutes = time_part[2:4]
    seconds = time_part[4:6]

    zona_horaria = pytz.timezone('Europe/Madrid')
    now = datetime.now(zona_horaria)
    timestamp = now.strftime('%Y-%m-%d %H:%M:%S CEST%z')

    data = {
        "name": "irivera",
        "password": API_PASSWORD,
        "device_id": DEVICE_ID,
        "timestamp": timestamp,
        "segundos": int(minutes + seconds),
        "temperatura": temp,
        "humedad": humid,
        "bateriaArduino": bateriaArduino,
        "bateriaPi": bateriaPi
    }

    response = requests.post(MONITORING_URL, json=data, headers={"Content-Type": "application/json"})
    
    if response.status_code == 200:
        print("Datos de monitorización enviados correctamente.")
        log_action("Monitorización: datos enviados correctamente.")
    else:
        print(f"Error al enviar datos de monitorización. Código: {response.status_code}")
        log_action(f"Monitorización: error al enviar datos. Código: {response.status_code}")

def main():
    filepath, filename = take_photo()
    log_action(f"Photo {filename} taken.")

    temp, humid = read_sensor_data()
    log_sensor_data(temp, humid)

    bateriaArduino, bateriaPi = read_battery_data()

    upload_to_server()

    send_monitoring_data(filename, temp, humid, bateriaArduino, bateriaPi)

    delete_photos()

    #shutdown_system()

if __name__ == "__main__":
    main()
