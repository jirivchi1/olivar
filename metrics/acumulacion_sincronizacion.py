import re
from datetime import datetime
import os

# Definir el archivo de salida
output_file = "output.txt"

# Definir umbrales (si decides implementarlos en el futuro)
MIN_THRESHOLD = 3500  # segundos
MAX_THRESHOLD = 3900  # segundos

# Paso 1: Cargar log_banda.txt
with open("log_banda.txt", "r") as file:
    lines = file.readlines()

# Paso 2: Extraer las líneas relevantes (aquellas con "Photo")
photo_lines = [line.strip() for line in lines if "Photo" in line]

# Paso 3: Extraer la hora y el nombre de la foto
time_format = "%Y-%m-%d %H:%M:%S.%f"
photo_data = []

for line in photo_lines:
    # Extraer hora y nombre de la foto usando regex
    match = re.match(
        r"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{6}): Photo (\S+) taken.", line
    )
    if match:
        try:
            time_str = match.group(1)
            photo_name = match.group(2)
            # Convertir la marca de tiempo a un objeto datetime
            photo_time = datetime.strptime(time_str, time_format)
            photo_data.append((photo_time, photo_name))
        except ValueError:
            # Si la conversión falla, simplemente omite la línea
            continue

# Paso 4: Calcular la diferencia de tiempo en segundos
differences = []
for i in range(1, len(photo_data)):
    time_diff = (photo_data[i][0] - photo_data[i - 1][0]).total_seconds()
    differences.append(time_diff)

# Paso 5: Escribir las diferencias en out.txt con las columnas especificadas
# Verificar si el archivo existe y si está vacío para escribir el encabezado
write_header = not os.path.isfile(output_file) or os.path.getsize(output_file) == 0

with open(output_file, "a") as out_file:
    if write_header:
        header = "T1\tT2\tT3\tT4\tT5\tT1-T2\tT1-T3\n"
        out_file.write(header)

    for i, diff in enumerate(differences):
        # Obtener la marca de tiempo de la foto anterior
        T1 = differences[i]

        # Como no tienes datos para T2, T3, T4, T5, se llenan con 'N/A'
        T2 = "N/A"
        T3 = "N/A"
        T4 = "N/A"
        T5 = "N/A"

        # Asignar la diferencia a T1-T2 y dejar T1-T3 como 'N/A'
        T1_T2 = "N/A"
        T1_T3 = "N/A"

        # Escribir la línea en el archivo de salida
        line = f"{T1}\t{T2}\t{T3}\t{T4}\t{T5}\t{T1_T2}\t{T1_T3}\n"
        out_file.write(line)

        # Opcional: Imprimir en consola
        print(
            f"Time difference between {photo_data[i][1]} and {photo_data[i+1][1]}: {diff} seconds"
        )
