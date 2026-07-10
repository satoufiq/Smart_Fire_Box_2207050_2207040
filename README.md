# 🔥 Smart Fire Box

**IoT-Based Intelligent Fire Detection & Suppression System**

An autonomous, multi-zone fire safety system built with dual ESP32 microcontrollers that detects fire, smoke, high temperature, and overcurrent — then automatically suppresses the fire with a servo-aimed water nozzle, activates exhaust ventilation, opens emergency exit doors, and reports everything to the cloud in real time.

---

## 📋 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Components](#components)
- [Pin Configuration](#pin-configuration)
- [Communication Protocol](#communication-protocol)
- [State Machine & Priority Logic](#state-machine--priority-logic)
- [Actuator Behavior](#actuator-behavior)
- [Blynk IoT Cloud Integration](#blynk-iot-cloud-integration)
- [Software Dependencies](#software-dependencies)
- [Setup & Flashing](#setup--flashing)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
- [Applications](#applications)
- [Limitations](#limitations)
- [License](#license)

---

## Overview

The **Smart Fire Box** is a prototype fire detection and suppression system designed for enclosed spaces such as rooms, server cabinets, warehouses, or laboratories. It uses **16 sensors across 4 independent zones** to monitor environmental conditions and a suite of actuators to respond autonomously to threats.

### Key Features

- 🔍 **4-Zone Detection** — Each zone has its own flame, smoke, temperature, and distance sensor  
- 🎯 **Automated Nozzle Targeting** — Pan Servos(Bottom Servo for Zone 1(0 degree), Zone 2(90 degree), Zone 3(180 degree) and top servo for zone 4(Bottom 180 degree + Top 90 degree)) mechanism aims the water nozzle at the exact fire zone  
- 💨 **Smoke Ventilation** — Exhaust fan activates automatically when smoke exceeds threshold  
- 🚪 **Emergency Exit Door** — PIR motion sensor triggers door opening during fire events  
- ⚡ **Overcurrent Protection** — ACS712 current sensor monitors for electrical faults  
- ☁️ **Real-Time Cloud Dashboard** — All 26 data points pushed to Blynk IoT every second  
- 🧠 **Dual-Core FreeRTOS** — Network communication on Core 0 never delays fire response on Core 1  

---

## System Architecture

The system is split into two independent ESP32 microcontrollers communicating over **UART (Serial2)**:

```
┌─────────────────────────┐       UART (9600 baud)       ┌─────────────────────────┐
│   ESP32 #1 — SENSOR     │ ──────────────────────────▶  │   ESP32 #2 — ACTUATOR   │
│                         │   TX(GPIO 15) → RX(GPIO 16)  │                         │
│  • 4× IR Flame Sensors  │                              │  • 3× SG90 Servos       │
│  • 4× MQ-2 Smoke        │                              │  • 2× Relay (Fan, Pump) │
│  • 4× DHT11 Temp        │                              │  • Active Buzzer        │
│  • 4× HC-SR04 Distance  │                              │  • PIR Motion Sensor    │
│                         │                              │  • ACS712 Current Sensor│
│  Reads 16 sensors       │                              │  • Blynk Cloud (WiFi)   │
│  every 1 second         │                              │  • FreeRTOS Dual-Core   │
└─────────────────────────┘                              └─────────────────────────┘
```

**Why two ESP32s?**  
Sensor reading latency (especially DHT11 and ultrasonic timing) is completely isolated from actuator control and network communication, ensuring sub-second fire response times.

---

## Components

| Component | Quantity | Role |
|---|---|---|
| ESP32 DevKit V1 | 2 | Sensor unit + Actuator unit |
| IR Flame Sensor Module | 4 | Detects infrared radiation from fire |
| MQ-2 Gas/Smoke Sensor | 4 | Detects smoke, LPG, methane (analog) |
| DHT11 Temperature Sensor | 4 | Measures temperature per zone |
| HC-SR04 Ultrasonic Sensor | 4 | Measures distance / object presence |
| PIR Motion Sensor (HC-SR501) | 1 | Detects human presence for door |
| ACS712 Current Sensor | 1 | Monitors electrical current draw |
| SG90 Micro Servo Motor | 3 | Pan nozzle, tilt nozzle, door |
| 2-Channel Relay Module | 1 | Controls exhaust fan and water pump |
| Active Buzzer | 1 | Audible fire/overcurrent alarm |
| Mini Water Pump | 1 | Fire suppression spray |
| DC Exhaust Fan | 1 | Smoke ventilation |
| Breadboard & Jumper Wires | — | Prototyping connections |

---

## Pin Configuration

### ESP32 #1 — Sensor Unit

| Function | GPIO Pin(s) | Type |
|---|---|---|
| Flame Sensors (Zone 1–4) | 13, 14, 18, 23 | Digital Input |
| MQ-2 Smoke Sensors (Zone 1–4) | 34, 36, 32, 33 | Analog Input (ADC) |
| DHT11 Temp Sensors (Zone 1–4) | 4, 16, 19, 26 | Digital (1-Wire) |
| HC-SR04 TRIG (Zone 1–4) | 5, 17, 21, 27 | Digital Output |
| HC-SR04 ECHO (Zone 1–4) | 35, 39, 22, 25 | Digital Input |
| UART TX → ESP32 #2 RX | 15 | Serial2 TX |
| UART RX (unused for receive) | 12 | Serial2 RX |

### ESP32 #2 — Actuator Unit

| Function | GPIO Pin | Type |
|---|---|---|
| UART RX ← ESP32 #1 TX | 16 | Serial2 RX |
| UART TX (unused for send) | 17 | Serial2 TX |
| Bottom Servo (Pan) | 18 | PWM Output |
| Top Servo (Tilt) | 19 | PWM Output |
| Fan Relay | 21 | Digital Output (Active LOW) |
| Pump Relay | 22 | Digital Output (Active LOW) |
| Buzzer | 32 | Digital Output |
| Door Servo | 33 | PWM Output |
| PIR Motion Sensor | 13 | Digital Input |
| ACS712 Current Sensor | 34 | Analog Input (ADC) |

---

## Communication Protocol

ESP32 #1 transmits a structured data packet over UART every **1 second**:

```
Z1,flame,smoke,temp,dist;Z2,flame,smoke,temp,dist;Z3,flame,smoke,temp,dist;Z4,flame,smoke,temp,dist#
```

**Example packet:**
```
Z1,0,320,28.5,42;Z2,1,1850,45.2,15;Z3,0,180,27.0,38;Z4,0,290,26.8,50#
```

| Field | Type | Description |
|---|---|---|
| `ZX` | Zone ID | Zone number (1–4) |
| `flame` | `0` or `1` | `1` = fire detected (IR sensor LOW) |
| `smoke` | `0–4095` | Analog ADC value from MQ-2 |
| `temp` | `float` | Temperature in °C from DHT11 (`-999.0` if read fails) |
| `dist` | `int` | Distance in cm from HC-SR04 (`-1` if timeout) |

The `#` character terminates the packet. ESP32 #2 buffers incoming bytes and parses on `#`.

---

## State Machine & Priority Logic

The actuator unit evaluates threats using a **priority-based state machine** (highest priority first):

```
┌─────────────────┐
│  OVER_CURRENT   │  ← Highest Priority (ADC > 2600)
├─────────────────┤
│     FIRE        │  ← Any zone flame == 1
├─────────────────┤
│     SMOKE       │  ← Any zone smoke > 1000
├─────────────────┤
│   HIGH_TEMP     │  ← Any zone temp > 35°C
├─────────────────┤
│    NORMAL       │  ← No anomaly detected
└─────────────────┘
```

| State | Trigger Condition | Actions |
|---|---|---|
| `STATE_OVER_CURRENT` | Current sensor ADC > 2600 | Buzzer ON |
| `STATE_FIRE` | Any zone `flame == 1` | Buzzer ON, servo aims at zone, pump ON, door opens if motion |
| `STATE_SMOKE` | Any zone `smoke > 1000` | Exhaust fan ON |
| `STATE_HIGH_TEMP` | Any zone `temp > 35°C` | Early warning (reported to cloud) |
| `STATE_NORMAL` | No anomaly | All actuators OFF, door closed |

> **Note:** Smoke ventilation (fan) operates independently of the displayed state — if smoke is present during a fire, the fan still runs.

---

## Actuator Behavior

### Servo Nozzle Targeting

The pan/tilt servo mechanism aims the water nozzle at the detected fire zone:

| Zone | Bottom Servo (Pan) | Top Servo (Tilt) |
|---|---|---|
| Zone 1 | 0° | 0° |
| Zone 2 | 180° | 90° |
| Zone 3 | 180° | 0° |
| Zone 4 | 90° | 0° |

A **1.5-second delay** is applied when switching between zones to allow servos to reach position before activating the pump.

### Emergency Door

- **Opens** (90°) when fire is active AND PIR detects motion (person present)  
- **Closes** (0°) automatically when no fire is active  

### Relay Logic

Both relays use **active-LOW** logic:
- `LOW` = Relay ON (circuit closed)
- `HIGH` = Relay OFF (circuit open)

---

## Blynk IoT Cloud Integration

All sensor data and actuator states are published to the **Blynk IoT** platform over WiFi using **26 virtual pins**:

| Virtual Pin | Data |
|---|---|
| V0–V3 | Zone 1: Temperature, Smoke, Flame, Distance |
| V4–V7 | Zone 2: Temperature, Smoke, Flame, Distance |
| V8–V11 | Zone 3: Temperature, Smoke, Flame, Distance |
| V12–V15 | Zone 4: Temperature, Smoke, Flame, Distance |
| V16 | Pump Status (1 = ON) |
| V17 | Motion Detected (1 = YES) |
| V18 | Overcurrent Alarm (1 = YES) |
| V19 | Nozzle Servo Angle |
| V20 | Fan Status (1 = ON) |
| V21 | Door Status (1 = OPEN) |
| V22 | System Status String |
| V23 | Active Fire Zone (0 = None) |
| V24 | Alarm Message String |
| V25 | Buzzer Status (1 = ON) |

Blynk runs on **Core 0** via FreeRTOS `xTaskCreatePinnedToCore`, ensuring that WiFi reconnection and cloud sync never block the main fire-response loop on **Core 1**.

---

## Software Dependencies

### Arduino Libraries

| Library | Purpose |
|---|---|
| `DHT.h` | DHT11 temperature sensor driver |
| `WiFi.h` | ESP32 WiFi connectivity |
| `BlynkSimpleEsp32.h` | Blynk IoT cloud integration |
| `ESP32Servo.h` | Servo motor control for ESP32 |

### Platform

- **Board:** ESP32 DevKit V1  
- **Framework:** Arduino (via Arduino IDE or PlatformIO)  
- **Baud Rate:** 9600 (UART between ESP32s), 115200 (debug serial)

---

## Setup & Flashing

### 1. Install Arduino IDE

Download from [arduino.cc](https://www.arduino.cc/en/software) and add ESP32 board support:
```
File → Preferences → Additional Board URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### 2. Install Required Libraries

Via **Library Manager** (`Sketch → Include Library → Manage Libraries`):
- `DHT sensor library` by Adafruit
- `Blynk` by Volodymyr Shymanskyy
- `ESP32Servo` by Kevin Harrington

### 3. Configure Blynk Credentials

In `actuator.ino`, update these lines with your own Blynk project credentials:
```cpp
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Fire Box"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
```

### 4. Flash the Firmware

1. Connect **ESP32 #1** via USB → Select correct COM port → Upload `sensor_sender_esp32.ino`
2. Connect **ESP32 #2** via USB → Select correct COM port → Upload `actuator.ino`

### 5. Wire the Hardware

Connect all sensors and actuators according to the [Pin Configuration](#pin-configuration) tables above. Ensure:
- UART TX (GPIO 15) of ESP32 #1 is connected to UART RX (GPIO 16) of ESP32 #2
- Both ESP32s share a common **GND**
- Relays are powered from a separate 5V supply if needed

---

## Project Structure

```
IOT/
├── sensor_sender_esp32/
│   ├── sensor_sender_esp32.ino   # ESP32 #1 — Reads 16 sensors, sends UART packets
│   └── actuator.ino              # ESP32 #2 — Receives data, controls actuators, Blynk
├── Smart_Fire_Box_Project_Report.docx   # Detailed project report
├── README.md                            # This file
└── *.jpeg                               # Project photographs
```

---

## How It Works

```
  ┌──────────┐    1. Read 16 sensors     ┌──────────┐    3. Evaluate state     ┌──────────┐
  │  SENSORS │ ─────────────────────────▶ │   UART   │ ──────────────────────▶ │ ACTUATOR │
  │ (ESP32#1)│    every 1 second          │  PACKET  │    priority logic       │ (ESP32#2)│
  └──────────┘                            └──────────┘                         └─────┬────┘
                                                                                     │
                                          ┌──────────┐    4. Push 26 data      ┌─────▼────┐
                                          │  BLYNK   │ ◀────────────────────── │  CLOUD   │
                                          │  MOBILE  │    points every 1s      │  SYNC    │
                                          │   APP    │                         │ (Core 0) │
                                          └──────────┘                         └──────────┘
```

1. **Sense** — ESP32 #1 reads flame (digital), smoke (analog), temperature (DHT11), and distance (ultrasonic) from all 4 zones every second.
2. **Transmit** — Sensor data is packed into a structured UART string and sent to ESP32 #2.
3. **Decide** — ESP32 #2 parses the packet, reads PIR and current sensors locally, then evaluates the highest-priority threat using the state machine.
4. **Act** — Appropriate actuators are triggered: buzzer, servo nozzle, water pump, exhaust fan, and/or emergency door.
5. **Report** — All 26 data points are synced to the Blynk IoT cloud dashboard for remote monitoring via mobile app.

---

## Applications

- 🏠 Residential fire safety — apartments, kitchens, garages  
- 🖥️ Server rooms & data centers — protects critical IT infrastructure  
- 🏭 Warehouses & factories — zone-based industrial monitoring  
- 🔬 Laboratories — chemical and electrical fire risk environments  
- ⚡ Electrical panels — overcurrent detection prevents electrical fires  
- 🏛️ Museums & archives — early detection protects irreplaceable items  
- 🤖 Unmanned facilities — fully autonomous fire response  
- 🏡 Smart home integration — extends into broader IoT ecosystems  

---

## Limitations

| Limitation | Potential Improvement |
|---|---|
| Cardboard enclosure (prototype) | Fire-resistant metal/ABS housing |
| Limited water reservoir | Larger tank or mains water connection |
| Single nozzle (one zone at a time) | Multiple nozzles or faster servo |
| DHT11 accuracy (±2°C, slow) | Upgrade to DHT22 or thermocouple |
| MQ-2 requires ~20s warm-up | Pre-heat cycle on boot |
| WiFi dependency for cloud | Add offline SD card logging |
| No battery backup | Add UPS / battery failover |
| SG90 limited torque | Upgrade to MG996R for larger builds |
| No camera integration | Add ESP32-CAM for visual confirmation |
| UART is point-to-point | Scale with ESP-NOW or I²C bus |

---

## License

This project was developed as an academic/educational IoT prototype. Feel free to use, modify, and build upon it for learning and non-commercial purposes.

---

> **Smart Fire Box** — Because every second counts when fire strikes. 🔥🛡️
