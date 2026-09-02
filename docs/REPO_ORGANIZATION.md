# Smart Fire Box

This repository contains a dual-ESP32 fire detection and response system with:

- a **sensor unit** for multi-zone sensing
- an **actuator unit** for hazard response and IoT monitoring
- a **project report**
- a **demo video / visual evidence** folder

## Suggested layout

```text
Smart_Fire_Box_2207050_2207040/
├── code/
│   ├── sensor-unit/
│   │   └── ESP 32#1, SENSOR Unit.ino
│   └── actuator-unit/
│       └── esp 32 #2 actuator unit.ino
├── report/
│   └── Smart_Fire_Box__Report.pdf
├── media/
│   └── demo/
│       └── Visuals - Demo Video and Pictures/
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
The visuals folder contains the demo video and supporting images.

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