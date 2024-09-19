# Olivar Project

## Table of Contents
- [About](#about)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Features](#features)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## About
The Olivar project is an innovative decision support system for Integrated Pest Management (IPM) in olive groves. It uses artificial vision technology to monitor the daily flight curve of olive pests such as (Mosca de olivo, Prais, Abichado y Glifodes). The main goal is to provide daily information on the Economic Damage Threshold (UDE) to anticipate treatments and minimize the use of phytosanitary products.

## Getting Started

### Prerequisites
- Arduino board
- Raspberry Pi
- SHT20 temperature and humidity sensor
- Webcam
- Python 3.x
- Arduino IDE

### Installation
1. Clone the repository:
   ```
   git clone https://github.com/jirivchi1/olivar.git
   ```
2. Upload `arduino_code/main.ino` to your Arduino board using the Arduino IDE.
3. Install required Python packages on the Raspberry Pi:
   ```
   pip3 install sht20
   ```
4. Install `fswebcam` on the Raspberry Pi:
   ```
   sudo apt-get install fswebcam
   ```

## Usage
1. Configure the server details in the Python scripts (`SERVER_USER`, `SERVER_IP`, `SERVER_DIR`).
2. Run the appropriate node script on the Raspberry Pi:
   ```
   python nodo_banda/main.py
   ```
3. The system will automatically capture images and sensor data, uploading them to the specified server.

## Project Structure
```
├── arduino_code
│   └── main.ino
├── nodo_amarillo
│   ├── main.py
│   └── photo_count.txt
├── nodo_banda
│   └── main.py
├── nodo_verde
│   └── main.py
└── README.md
```

## Features
- Automated image capture using webcam
- Temperature and humidity monitoring with SHT20 sensor
- Periodic data upload to remote server
- Low-power operation for Arduino-controlled components
- Configurable day/night cycles for pest monitoring

## Contributing
Contributions to the Olivar project are welcome. Please follow these steps:
1. Fork the repository
2. Create a new branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request


## Contact
Ismael Rivera - jirivera@uloyola.e

