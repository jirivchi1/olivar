import serial
import time

# Configura la conexin serial con el Arduino
ser = serial.Serial("/dev/ttyS0", 9600, timeout=1)
time.sleep(2)  # Espera a que la conexin se establezca

# Obtiene la hora actual
current_hour = time.strftime("%H")
print(current_hour)
# Envia la hora al Arduino
ser.write(current_hour.encode())
