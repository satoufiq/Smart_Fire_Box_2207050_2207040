# Smart Fire Box

An ESP32-based smart fire detection and response system built around two coordinated units:

- **ESP32 #1 — Sensor Unit**: collects fire-related readings from four zones using flame sensors, MQ-2 smoke sensors, DHT11 temperature sensors, and ultrasonic distance sensors.
- **ESP32 #2 — Actuator Unit**: receives the telemetry, detects hazards, and controls the pump, fan, door servo, fire-targeting servos, buzzer, and Blynk IoT dashboard.

This project demonstrates a practical multi-zone safety system for early fire detection, localization, emergency response, and remote monitoring.

## Repository Contents

| Folder / File | Description |
|---|---|
| `Sensor unit code/ESP 32#1, SENSOR Unit.ino` | Sensor unit firmware for reading all 4 zones and sending UART packets. |
| `Actuator unit code/esp 32 #2 actuator unit.ino` | Actuator unit firmware for parsing sensor data and controlling response hardware. |
| `Report/Smart_Fire_Box__Report.pdf` | Project report and documentation. |
| `Demonstration Video/Smar Fire Box - Project Demonstration.mp4` | Demonstration video. |
| `Visuals/` | Supporting images and interface screenshots. |

## Project Overview

The Smart Fire Box is designed to monitor four fire-detection zones and automatically respond when a hazard is detected. The system uses a distributed architecture:

1. **Sensor Unit (ESP32 #1)** reads:
   - Flame sensor state
   - Smoke level from MQ-2
   - Temperature from DHT11
   - Distance from HC-SR04

2. **Actuator Unit (ESP32 #2)** receives packets over UART and decides the system state:
   - **NORMAL**: no hazard detected
   - **HIGH TEMP**: elevated temperature detected
   - **SMOKE**: smoke level above threshold
   - **FIRE**: flame detected in a zone
   - **OVER CURRENT**: current sensor detects unsafe load

3. **Response hardware** is triggered as needed:
   - Water pump relay
   - Exhaust fan relay
   - Buzzer
   - Two-axis fire-targeting servos
   - Emergency door servo when motion is detected

4. **Blynk IoT dashboard** displays real-time telemetry and system status.

## System Features

- 4-zone fire monitoring
- Flame, smoke, temperature, and distance sensing
- UART-based communication between both ESP32 boards
- Automatic fire localization and suppression
- Smoke exhaust fan control
- Emergency door response based on PIR motion detection
- Overcurrent protection and alarm handling
- Blynk-based remote monitoring

## Hardware Architecture

### Sensor Unit

The sensor sketch uses the following pin assignments:

- **Zone 1**: DHT11 on GPIO 17, ultrasonic on GPIO 4/16, MQ-2 on GPIO 32, flame sensor on GPIO 5
- **Zone 2**: DHT11 on GPIO 18, ultrasonic on GPIO 19/21, MQ-2 on GPIO 39, flame sensor on GPIO 23
- **Zone 3**: DHT11 on GPIO 27, ultrasonic on GPIO 26/25, MQ-2 on GPIO 34, flame sensor on GPIO 22
- **Zone 4**: DHT11 on GPIO 33, ultrasonic on GPIO 14/13, MQ-2 on GPIO 35, flame sensor on GPIO 15

The sensor packet format sent over Serial is:

```text
Z1,<flame>,<smoke>,<temp>,<dist>;Z2,<flame>,<smoke>,<temp>,<dist>;Z3,<flame>,<smoke>,<temp>,<dist>;Z4,<flame>,<smoke>,<temp>,<dist>#
```

### Actuator Unit

The actuator sketch uses:

- **UART RX/TX**: GPIO 16 / GPIO 17
- **Bottom servo**: GPIO 18
- **Top servo**: GPIO 19
- **Fan relay**: GPIO 21
- **Pump relay**: GPIO 22
- **PIR sensor**: GPIO 13
- **Current sensor**: GPIO 34
- **Buzzer**: GPIO 32
- **Door servo**: GPIO 33

## How the System Works

### 1. Sensing
The sensor unit continuously reads each zone and builds a telemetry packet.

### 2. Transmission
Sensor data is sent every second through UART.

### 3. Decision Making
The actuator unit parses the packet and checks for:
- flame detection
- smoke thresholds
- high temperature
- overcurrent

### 4. Response
Depending on the detected state, the system may:
- activate the buzzer
- start the pump
- turn on the fan
- rotate the servo nozzle to the active zone
- open the emergency door if motion is detected during fire

### 5. Monitoring
The actuator unit mirrors data to the Blynk dashboard for live visualization.

## Setup Instructions

### Requirements
- Arduino IDE
- ESP32 board support package
- ESP32 DevKit boards
- DHT sensor library
- Adafruit Unified Sensor
- ESP32Servo
- Blynk library

### Installation
1. Open `Sensor unit code/ESP 32#1, SENSOR Unit.ino` and `Actuator unit code/esp 32 #2 actuator unit.ino` in Arduino IDE.
2. Install the required libraries.
3. Configure Wi-Fi and Blynk credentials in the actuator sketch.
4. Connect ESP32 #1 TX to ESP32 #2 RX.
5. Share a common ground between all modules.
6. Upload the sensor sketch to ESP32 #1 and the actuator sketch to ESP32 #2.

## Repository Structure

```text
Smart_Fire_Box_2207050_2207040/
├── README.md
├── Sensor unit code/
│   └── ESP 32#1, SENSOR Unit.ino
├── Actuator unit code/
│   └── esp 32 #2 actuator unit.ino
├── Report/
│   └── Smart_Fire_Box__Report.pdf
├── Demonstration Video/
│   └── Smar Fire Box - Project Demonstration.mp4
└── Visuals/
    ├── Blynk Interface - Datastreams (1-14).jpeg.png
    ├── Blynk Interface - Datastreams (15-25).jpeg.png
    ├── Blynk Interface -Device Dashboard.jpeg.png
    ├── Blynk Interface -Events and Notifications.jpeg.png
    ├── Side view of Zone 2 — water pump container, sensor wiring, and exhaust fan.jpeg
    ├── Top-down view — two ESP32 DevKit V1 boards on breadboards with relay module and wiring.jpeg
    ├── Underside view — DHT11, HC-SR04, flame sensor, and SG90 nozzle servo mechanism.jpeg
    └── Zone 1 front panel — emergency exit door servo, HC-SR04, flame sensor, MQ-2, and exhaust fan.jpg
```

## Notes

- The actuator sketch currently contains Wi-Fi and Blynk credentials in plain text. These should be replaced with placeholders before public sharing.

## Demonstration Video

The demonstration video is available in `Demonstration Video/Smar Fire Box - Project Demonstration.mp4`.
Supporting images are available in the `Visuals/` folder.

## Report

The project report is included as `Report/Smart_Fire_Box__Report.pdf`.

## License

No license was provided in the repository.

## Acknowledgements

This project was developed as part of the Smart Fire Box coursework/project submission.