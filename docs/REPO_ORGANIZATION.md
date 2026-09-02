# Smart Fire Box

This repository contains a dual-ESP32 fire detection and response system with:

- a **sensor unit** for multi-zone sensing
- an **actuator unit** for hazard response and IoT monitoring
- a **project report**
- a **demo video / visual evidence** folder

## Repository layout

```text
Smart_Fire_Box_2207050_2207040/
├── Sensor unit code/
│   └── ESP 32#1, SENSOR Unit.ino
├── Actuator unit code/
│   └── esp 32 #2 actuator unit.ino
├── Report/
│   └── Smart_Fire_Box__Report.pdf
├── Demonstration Video/
│   └── Smar Fire Box - Project Demonstration.mp4
├── Visuals/
│   ├── Blynk Interface - Datastreams (1-14).jpeg.png
│   ├── Blynk Interface - Datastreams (15-25).jpeg.png
│   ├── Blynk Interface -Device Dashboard.jpeg.png
│   ├── Blynk Interface -Events and Notifications.jpeg.png
│   ├── Side view of Zone 2 — water pump container, sensor wiring, and exhaust fan.jpeg
│   ├── Top-down view — two ESP32 DevKit V1 boards on breadboards with relay module and wiring.jpeg
│   ├── Underside view — DHT11, HC-SR04, flame sensor, and SG90 nozzle servo mechanism.jpeg
│   └── Zone 1 front panel — emergency exit door servo, HC-SR04, flame sensor, MQ-2, and exhaust fan.jpg
└── README.md
```

## Contents

### 1. Actuator Unit Code
Controls:
- fire localization servos
- water pump relay
- exhaust fan relay
- buzzer
- emergency door servo
- PIR and current sensor logic
- Blynk telemetry

### 2. Sensor Unit Code
Reads four zones of:
- flame sensor
- MQ-2 smoke sensor
- DHT11 temperature sensor
- ultrasonic distance sensor

Sends packet format:

```text
Z1,<flame>,<smoke>,<temp>,<dist>;Z2,<flame>,<smoke>,<temp>,<dist>;Z3,<flame>,<smoke>,<temp>,<dist>;Z4,<flame>,<smoke>,<temp>,<dist>#
```

### 3. Report
The PDF report documents the project design, implementation, and results.

### 4. Demonstration Video
The demonstration video is in `Demonstration Video/`.

### 5. Visuals
`Visuals/` contains image files only.

## Setup
1. Install Arduino IDE and ESP32 board support.
2. Install required libraries:
   - DHT sensor library
   - Adafruit Unified Sensor
   - ESP32Servo
   - Blynk
3. Upload the sensor code to ESP32 #1.
4. Upload the actuator code to ESP32 #2.
5. Connect ESP32 #1 TX to ESP32 #2 RX and share ground.

## Security note
The actuator sketch includes Wi-Fi/Blynk credentials. Replace them with placeholders before publishing publicly.