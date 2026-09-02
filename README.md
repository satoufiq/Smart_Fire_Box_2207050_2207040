# Smart Fire Box

A dual-ESP32 smart fire detection and response system with a dedicated sensor node and actuator node.

## Project Overview

The project monitors four zones using flame, smoke, temperature, and distance sensors, then automatically activates mitigation components such as the pump, fan, buzzer, and servos when hazards are detected. Telemetry is exchanged between the two ESP32 boards via UART and visualized through Blynk.

## Repository Structure

```text
Smart_Fire_Box_2207050_2207040/
├── Actuator unit code/
│   └── esp 32 #2 actuator unit.ino
├── Sensor unit code/
│   └── ESP 32#1, SENSOR Unit.ino
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

## Main Components

- **Sensor Unit (`Sensor unit code/`)**: reads zone-wise flame, smoke (MQ-2), temperature (DHT11), and distance (HC-SR04) data.
- **Actuator Unit (`Actuator unit code/`)**: parses incoming sensor packets and controls fan, pump, buzzer, and servo mechanisms.
- **Report (`Report/`)**: complete project documentation.
- **Demonstration Video (`Demonstration Video/`)**: project demo video.
- **Visuals (`Visuals/`)**: supporting images and interface snapshots.

## Setup Notes

1. Open both `.ino` sketches in Arduino IDE.
2. Install required libraries used by the sketches (ESP32 core, DHT, Adafruit Unified Sensor, ESP32Servo, Blynk).
3. Configure board-specific credentials/settings as needed.
4. Upload the sensor sketch to ESP32 #1 and actuator sketch to ESP32 #2.

## License

No license file is currently included in this repository.
