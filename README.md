# Smart Fire Box

An IoT-based intelligent fire detection, localization, and automated suppression system built with dual ESP32 microcontrollers[span_0](start_span)[span_0](end_span). The system continuously monitors 4 separate zones using a multi-sensor array, transmits telemetry across UART[span_1](start_span)[span_1](end_span), controls targeted suppression mechanisms via a 2-DOF servo nozzle and pump[span_2](start_span)[span_2](end_span), handles automated emergency evacuation doors via PIR sensing[span_3](start_span)[span_3](end_span), and syncs live analytics with Blynk IoT using dual-core FreeRTOS task scheduling[span_4](start_span)[span_4](end_span).

---

## Key Features

* **Multi-Zone Sensory Array (4 Zones):** Real-time monitoring of Flame, Smoke (MQ-2), Temperature/Humidity (DHT11), and Distance (HC-SR04) across four zones[span_5](start_span)[span_5](end_span).
* **Dual ESP32 Distributed Architecture:**
  * **ESP32 #1 (Sensor Unit):** Reads 16 data points from 4 zones, serializes them into custom UART packets, and transmits every 1000 ms[span_6](start_span)[span_6](end_span).
  * **ESP32 #2 (Actuator Unit):** Parses UART packets, assesses hazard levels, drives relays, directs 3 servos, monitors overcurrent, and streams telemetry to Blynk IoT[span_7](start_span)[span_7](end_span).
* **2-DOF Targeted Fire Suppression:** Automatically directs a 2-axis servo water nozzle directly at the detected zone and activates the water pump[span_8](start_span)[span_8](end_span).
* **Smart Emergency Evacuation:** Opens the emergency exit door when human motion is detected via PIR during fire events[span_9](start_span)[span_9](end_span).
* **Smoke Exhaust System:** Automatically turns on the exhaust fan relay when elevated smoke levels are registered[span_10](start_span)[span_10](end_span).
* **Overcurrent & Electrical Hazard Protection:** Integrated current sensor triggers an overcurrent alarm and buzzer if current spikes above safe thresholds[span_11](start_span)[span_11](end_span).
* **Real-time IoT Dashboard:** Non-blocking Blynk IoT integration pinned to ESP32 Core 0 via FreeRTOS to maintain deterministic, zero-latency safety execution on Core 1[span_12](start_span)[span_12](end_span).

---

## System Architecture & Data Flow

```text
+--------------------------------------------------------------+
|                    ESP32 #1 - Sensor Unit                    |
|  [Zone 1..4: DHT11, MQ-2, Flame Sensor, HC-SR04 Sonar]       |
+------------------------------+-------------------------------+
                               | UART (9600 Baud)
                               v
+--------------------------------------------------------------+
|                   ESP32 #2 - Actuator Unit                   |
|  Core 1: Decision Logic, Relays, Servos, Buzzer, PIR Sensor  |
|  Core 0: FreeRTOS Task for Non-blocking Blynk IoT Sync       |
+------------------------------+-------------------------------+
                               |
       +-----------------------+-----------------------+
       v                                               v
[Hardware Actuators]                            [Blynk Cloud]
- 2-DOF Nozzle Servos                           - 16x Zone Data Feeds
- Emergency Exit Door Servo                     - Real-time Relays/Status
- Water Pump & Fan Relays                       - Active Alarms & Alerts
- Audio Buzzer

Hardware Components
 * Microcontrollers: 2x ESP32 DevKit V1
 * Sensors:
   * 4x DHT11 (Temperature & Humidity)
   * 4x MQ-2 (Smoke / Gas)
   * 4x IR Flame Sensors
   * 4x HC-SR04 Ultrasonic Sensors
   * 1x PIR Motion Sensor
   * 1x Analog Current Sensor
 * Actuators:
   * 3x SG90 / MG996R Servos (Bottom Pan, Top Tilt, Emergency Door)
   * 2x 5V Relay Modules (Submersible Water Pump, DC Exhaust Fan)
   * 1x Active 5V Buzzer
 * Power: 5V/12V DC External Power Supply
Pin Configurations
ESP32 #1: Sensor Transmitter
| Pin Type / Zone | Zone 1 | Zone 2 | Zone 3 | Zone 4 |
|---|---|---|---|---|
| DHT11 Data | GPIO 17 | GPIO 18 | GPIO 27 | GPIO 33 |
| MQ-2 Smoke (Analog) | GPIO 32 | GPIO 39 | GPIO 34 | GPIO 35 |
| Flame Sensor (Digital) | GPIO 5 | GPIO 23 | GPIO 22 | GPIO 15 |
| HC-SR04 Trig | GPIO 4 | GPIO 19 | GPIO 26 | GPIO 14 |
| HC-SR04 Echo | GPIO 16 | GPIO 21 | GPIO 25 | GPIO 13 |
| UART TX (To ESP32 #2) | GPIO 1 (Default TX0) |  |  |  |
ESP32 #2: Actuator & IoT Controller
| Component | ESP32 Pin | Logic / Type |
|---|---|---|
| UART RX2 / TX2 | GPIO 16 (RX), GPIO 17 (TX) | Serial2 (9600 Baud) |
| Bottom Pan Servo | GPIO 18 | PWM |
| Top Tilt Servo | GPIO 19 | PWM |
| Door Servo | GPIO 33 | PWM |
| Fan Relay | GPIO 21 | Active LOW |
| Pump Relay | GPIO 22 | Active LOW |
| Buzzer | GPIO 32 | Active HIGH |
| PIR Motion Sensor | GPIO 13 | Digital Input |
| Current Sensor | GPIO 34 | Analog Input |
UART Protocol Specification
Telemetry packets sent from ESP32 #1 to ESP32 #2 follow this delimited structure:
Z1,<flame>,<smoke>,<temp>,<dist>;Z2,<flame>,<smoke>,<temp>,<dist>;Z3,<flame>,<smoke>,<temp>,<dist>;Z4,<flame>,<smoke>,<temp>,<dist>#

 * Delimiters: ; separates zone data chunks; # marks the end of packet.
 * Example Packet:
   Z1,0,320,28.5,42;Z2,1,1240,46.2,12;Z3,0,290,27.8,50;Z4,0,310,28.0,48#

Blynk Virtual Pin Mapping
| Virtual Pin | Parameter | Type |
|---|---|---|
| V0 - V3 | Zone 1: Temperature, Smoke, Flame, Distance | Float / Integer |
| V4 - V7 | Zone 2: Temperature, Smoke, Flame, Distance | Float / Integer |
| V8 - V11 | Zone 3: Temperature, Smoke, Flame, Distance | Float / Integer |
| V12 - V15 | Zone 4: Temperature, Smoke, Flame, Distance | Float / Integer |
| V16 | Water Pump Status (1 = ON, 0 = OFF) | Integer |
| V17 | PIR Motion Detection Status | Integer |
| V18 | Overcurrent Alarm Status | Integer |
| V19 | Current Servo Angle | Integer |
| V20 | Exhaust Fan Status | Integer |
| V21 | Emergency Door Status (1 = Open, 0 = Closed) | Integer |
| V22 | System Status String (NORMAL, FIRE, SMOKE, etc.) | String |
| V23 | Active Fire Zone ID (1 - 4, 0 = None) | Integer |
| V24 | Dynamic Alarm Message | String |
| V25 | Buzzer Status | Integer |
Installation & Setup
 * Install Arduino IDE & ESP32 Board Core:
   * Install the ESP32 board definitions in Arduino IDE Boards Manager.
 * Install Required Libraries:
   * DHT sensor library by Adafruit
   * Adafruit Unified Sensor
   * ESP32Servo by Kevin Harrington
   * Blynk by Volodymyr Shymanskyy
 * Configure Actuator Code (actuator.ino):
   * Update the Wi-Fi credentials:
     char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

   * Update your Blynk credentials:
     #define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Fire Box"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

 * Wiring & Flashing:
   * Connect ESP32 #1 TX0 (GPIO 1) to ESP32 #2 RX2 (GPIO 16).
   * Ensure a common GND connection between both ESP32 units and external power modules.
   * Flash the respective sketches onto both ESP32 units.

