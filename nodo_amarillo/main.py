import os
import time
from datetime import datetime
import subprocess

# Configuración del servidor
SERVER_USER = "root"
SERVER_IP = "93.93.118.40"
SERVER_DIR = "/dataimages_olivar/trampa_amarilla_1"
LOCAL_DIRECTORY = "/home/pi/pruebas_campo/olivar/nodo_amarillo/fotos"
METRICS_DIRECTORY = "/home/pi/pruebas_campo/olivar/metrics"

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
    with open(f"{METRICS_DIRECTORY}/log_amarillo.txt", "a") as log_file:
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
    # Tomar la foto
    filepath, filename = take_photo()
    log_action(f"Photo {filename} taken.")
    
    # subir todas las fotos y datos del sensor al servidor
    #log_action("Uploading all photos and sensor data to server.")
    upload_to_server()

    # dele photos
    delete_photos()
    
    # Apagar el sistema
    # log_action("Shutting down the system.")
    shutdown_system()


if __name__ == "__main__":
    #time.sleep(15)
    main()
