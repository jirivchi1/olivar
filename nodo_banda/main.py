import os
import time
from datetime import datetime
import subprocess

# Configuración del servidor
SERVER_USER = "root"
SERVER_IP = "93.93.118.40"
SERVER_DIR = "/dataimages_olivar/trampa_banda"
LOCAL_DIRECTORY = "/home/pi/pruebas_campo/olivar/nodo_banda/fotos"
PHOTO_COUNT_FILE = "/home/pi/pruebas_campo/olivar/nodo_banda/photo_count.txt"
SENSOR_DATA_FILE = "/home/pi/pruebas_campo/olivar/nodo_banda/datos_sensor.txt"



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


def log_sensor_data(temp, humid):
    # Guardar la fecha, hora, temperatura y humedad en el archivo local
    with open(SENSOR_DATA_FILE, "a") as file:
        file.write(
            f"{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}, T: {temp}, H: {humid}\n"
        )


def upload_to_server():
    # Subir todas las fotos en el directorio LOCAL_DIRECTORY al servidor
    for filename in os.listdir(LOCAL_DIRECTORY):
        filepath = os.path.join(LOCAL_DIRECTORY, filename)
        if filename.endswith(".jpg"):
            subprocess.run(["scp", filepath, f"{SERVER_USER}@{SERVER_IP}:{SERVER_DIR}"])
            print(f"Image {filename} uploaded to the server.")

    # Subir los datos del sensor
    subprocess.run(
        [
            "scp",
            SENSOR_DATA_FILE,
            f"{SERVER_USER}@{SERVER_IP}:{SERVER_DIR}/datos_sensor.txt",
        ]
    )
    print("Sensor data uploaded to the server.")

    print("All images and sensor data uploaded.")


def delete_photos():
    # Eliminar todas las fotos en el directorio LOCAL_DIRECTORY
    for filename in os.listdir(LOCAL_DIRECTORY):
        filepath = os.path.join(LOCAL_DIRECTORY, filename)
        if filename.endswith(".jpg"):
            os.remove(filepath)
            print(f"Image {filename} deleted.")
    print("All images deleted.")


def log_action(message):
    with open(f"{LOCAL_DIRECTORY}/log.txt", "a") as log_file:
        log_file.write(f"{datetime.now()}: {message}\n")


def shutdown_system():
    subprocess.run(["sudo", "shutdown", "-h", "now"])


def read_photo_count():
    if os.path.exists(PHOTO_COUNT_FILE):
        with open(PHOTO_COUNT_FILE, "r") as file:
            return int(file.read())
    else:
        return 0


def write_photo_count(count):
    with open(PHOTO_COUNT_FILE, "w") as file:
        file.write(str(count))


def main():
    # Leer el contador actual
    photo_count = read_photo_count()

    # Tomar la foto
    filepath, filename = take_photo()
    log_action(f"Photo {filename} taken.")


    # Incrementar el contador  
    write_photo_count(photo_count)

    # Si se han tomado 4 fotos, subir todas las fotos y datos del sensor al servidor
    log_action("Uploading all photos and sensor data to server.")
    upload_to_server()

    # Apagar el sistema
    log_action("Shutting down the system.")
    shutdown_system()


if __name__ == "__main__":
    time.sleep(480)
    main()
