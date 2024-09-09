import os
import time
from datetime import datetime
import subprocess
import serial

# Configuración del servidor
SERVER_USER = "root"
SERVER_IP = "93.93.118.40"
SERVER_DIR = "/dataimages_olivar"
LOCAL_DIRECTORY = "/home/pi/Desktop/pruebas_campo"

# Configuración del puerto serial para Arduino
SERIAL_PORT = "/dev/ttyS0"
BAUD_RATE = 9600


def setup_serial():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Espera a que la conexión se establezca
    return ser


def send_time_to_arduino(ser):
    current_hour = time.strftime("%H")
    ser.write(current_hour.encode())


def is_daytime():
    current_hour = int(time.strftime("%H"))
    return 6 <= current_hour < 20  # Considera día entre las 6:00 y las 19:59


def take_photo():
    os.makedirs(LOCAL_DIRECTORY, exist_ok=True)
    filename = datetime.now().strftime("%Y%m%d_%H%M%S") + ".jpg"
    filepath = f"{LOCAL_DIRECTORY}/{filename}"
    subprocess.run(["fswebcam", "-r", "1280x720", "--no-banner", filepath])
    return filepath, filename


def upload_to_server(filepath, filename):
    subprocess.run(["scp", filepath, f"{SERVER_USER}@{SERVER_IP}:{SERVER_DIR}"])
    print(f"Image {filename} uploaded to the server.")


def log_action(message):
    with open(f"{LOCAL_DIRECTORY}/log.txt", "a") as log_file:
        log_file.write(f"{datetime.now()}: {message}\n")


def shutdown_system():
    subprocess.run(["sudo", "shutdown", "-h", "now"])


def main():
    ser = setup_serial()
    send_time_to_arduino(ser)

    if is_daytime():
        filepath, filename = take_photo()
        log_action(f"Photo taken: {filename}")
        upload_to_server(filepath, filename)
        log_action(f"Photo uploaded: {filename}")
    else:
        log_action("Nighttime - no photo taken")

    ser.close()
    log_action("System shutting down")
    shutdown_system()


if __name__ == "__main__":
    time.sleep(30)  # Espera inicial para que el sistema se inicialice completamente
    main()
