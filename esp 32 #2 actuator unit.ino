/*
 * SMART FIRE BOX PROJECT - ESP32 #2 (ACTUATOR UNIT)
 * 
 * Receives sensor data via UART and controls all actuators:
 * - 3x Servos (Bottom, Top, Door)
 * - 2x Relays (Fan, Pump)
 * - Buzzer
 * - PIR motion sensor
 * - Current Sensor
 */

#define BLYNK_TEMPLATE_ID "TMPL6_cwACJRd"
#define BLYNK_TEMPLATE_NAME "Smart Fire Box"
#define BLYNK_AUTH_TOKEN "f4BI39vfC6WkN08KEkai2V9IoGcXhT3r"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

char ssid[] = "Prottoy";
char pass[] = "987654321";

BlynkTimer timer;

//========================= PIN MAPPINGS =========================
#define RX_PIN 16
#define TX_PIN 17

#define BOTTOM_SERVO_PIN 18
#define TOP_SERVO_PIN 19
#define FAN_RELAY_PIN 21
#define PUMP_RELAY_PIN 22
#define PIR_SENSOR_PIN 13
#define CURRENT_SENSOR_PIN 34
#define BUZZER_PIN 32
#define DOOR_SERVO_PIN 33

//========================= SERVO OBJECTS =========================
Servo bottomServo;
Servo topServo;
Servo doorServo;

//========================= DATA STRUCTURES =========================
struct ZoneData {
    int flame;
    int smoke;
    float temperature;
    int distance;
};

ZoneData zones[4] = {
    {0, 0, 0.0, 0},
    {0, 0, 0.0, 0},
    {0, 0, 0.0, 0},
    {0, 0, 0.0, 0}
};

enum SystemState {
    STATE_NORMAL,
    STATE_HIGH_TEMP,
    STATE_SMOKE,
    STATE_FIRE,
    STATE_OVER_CURRENT
};

//========================= GLOBAL VARIABLES =========================
char rxBuffer[256];
int rxIndex = 0;
bool packetReady = false;

SystemState currentState = STATE_NORMAL;
int activeFireZone = -1;
int currentActiveZone = -1;
bool isDoorOpen = false;

bool isFanOn = false;
bool isPumpOn = false;
bool isBuzzerOn = false;
int currentAdcGlobal = 0;
String systemStatusStr = "NORMAL";
String alarmMessage = "";
bool isMotionDetected = false;

//========================= FUNCTION PROTOTYPES =========================
void receivePacket();
void parsePacket();
void processSystem();
void moveToZone(int zone);
bool openDoor();
bool closeDoor();
void pumpOn();
void pumpOff();
void sendBlynkData();

//========================= BLYNK TASK FOR CORE 0 =========================
TaskHandle_t blynkTask;

void blynkTaskCode(void* parameter) {
    for (;;) {
        if (WiFi.status() == WL_CONNECTED) {
            if (!Blynk.connected()) {
                Blynk.connect(); // Blocks ONLY Core 0, main loop stays fast!
            } else {
                Blynk.run();
            }
        }
        timer.run();
        delay(10); // Prevent watchdog timeout
    }
}

//========================= SETUP =========================
void setup() {
    Serial.begin(115200); // For debug monitoring
    Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // UART Communication with ESP32 #1

    // Initialize Relays (Active LOW)
    pinMode(FAN_RELAY_PIN, OUTPUT);
    digitalWrite(FAN_RELAY_PIN, HIGH); // OFF

    pinMode(PUMP_RELAY_PIN, OUTPUT);
    digitalWrite(PUMP_RELAY_PIN, HIGH); // OFF

    // Initialize Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // OFF

    // Initialize Sensors
    pinMode(CURRENT_SENSOR_PIN, INPUT);
    pinMode(PIR_SENSOR_PIN, INPUT);

    // Initialize Servos
    bottomServo.attach(BOTTOM_SERVO_PIN);
    topServo.attach(TOP_SERVO_PIN);
    doorServo.attach(DOOR_SERVO_PIN);

    currentActiveZone = -1;
    isDoorOpen = false;

    doorServo.write(0); // explicitly initialize door servo to 0
    bottomServo.write(0);
    topServo.write(0);

    // Connect to WiFi and configure Blynk (Non-blocking)
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    Blynk.config(BLYNK_AUTH_TOKEN);
    
    // Setup Blynk Timer to send data every 1 second
    timer.setInterval(1000L, sendBlynkData);
    
    // Launch Blynk strictly on Core 0 to guarantee ZERO latency on the main loop
    xTaskCreatePinnedToCore(
        blynkTaskCode, /* Task function */
        "BlynkTask",   /* Task name */
        10000,         /* Stack size */
        NULL,          /* Parameter */
        1,             /* Priority */
        &blynkTask,    /* Task handle */
        0);            /* Core 0 */
}

void loop() {
    
    receivePacket();
    
    if (packetReady) {
        parsePacket();
    }
    
    // Read PIR sensor
    isMotionDetected = digitalRead(PIR_SENSOR_PIN) == HIGH;
    
    processSystem();
    
    delay(50); // Small delay for loop stability
}

//========================= UART RECEIVE =========================
void receivePacket() {
    while (Serial2.available() > 0) {
        if (packetReady) {
            break; // Wait until packet is parsed before reading more
        }
        
        char c = Serial2.read();
        
        // Ignore carriage return and newline characters
        if (c == '\r' || c == '\n') {
            continue;
        }
        
        if (c == '#') {
            rxBuffer[rxIndex] = '\0'; // Null-terminate string
            packetReady = true;
            rxIndex = 0; // Reset index for next packet
            // Serial.println("Packet received");
        } else {
            if (rxIndex < sizeof(rxBuffer) - 1) {
                rxBuffer[rxIndex++] = c;
            }
        }
    }
}

//========================= PARSE PACKET =========================
void parsePacket() {
    // strtok modifies the buffer by placing null terminators at delimiters
    char* token = strtok(rxBuffer, ";");
    
    while (token != NULL) {
        int zoneNum, flame, smoke, dist;
        float temp;
        
        // Expected format per zone: ZX,flame,smoke,temp,dist (e.g., Z2,1,530,31.2,18)
        if (sscanf(token, "Z%d,%d,%d,%f,%d", &zoneNum, &flame, &smoke, &temp, &dist) == 5) {
            if (zoneNum >= 1 && zoneNum <= 4) {
                int index = zoneNum - 1;
                zones[index].flame = flame;
                zones[index].smoke = smoke;
                zones[index].temperature = temp;
                zones[index].distance = dist;
            }
        }
        token = strtok(NULL, ";");
    }
    
    packetReady = false; // Acknowledge parsing completion, ready for next packet
    
    // Serial.println("Parsed sensor values:");
    // for (int i = 0; i < 4; i++) {
    //     Serial.print("Zone "); Serial.print(i + 1);
    //     Serial.print(" | Flame: "); Serial.print(zones[i].flame);
    //     Serial.print(" | Smoke: "); Serial.print(zones[i].smoke);
    //     Serial.print(" | Temp: "); Serial.print(zones[i].temperature);
    //     Serial.print(" | Dist: "); Serial.println(zones[i].distance);
    // }
}

//========================= PROCESS SYSTEM =========================
void processSystem() {
    long sum = 0;
    for(int i = 0; i < 20; i++) {
        sum += analogRead(CURRENT_SENSOR_PIN);
    }
    int currentAdc = sum / 20;
    currentAdcGlobal = currentAdc; // Update global for Blynk
    bool overCurrent = (currentAdc > 2600);

    activeFireZone = -1;
    bool hasSmoke = false;
    bool hasHighTemp = false;

    // Evaluate all 4 zones
    for (int i = 0; i < 4; i++) {
        if (zones[i].flame == 1) {
            activeFireZone = i + 1; // Maps 0-3 to Zone 1-4
            break;
        }
        if (zones[i].smoke > 1000) {
            hasSmoke = true;
        }
        if (zones[i].temperature > 35.0) {
            hasHighTemp = true;
        }
    }

    // Determine highest priority state
    if (overCurrent) {
        currentState = STATE_OVER_CURRENT;
    } else if (activeFireZone != -1) {
        currentState = STATE_FIRE;
    } else if (hasSmoke) {
        currentState = STATE_SMOKE;
    } else if (hasHighTemp) {
        currentState = STATE_HIGH_TEMP;
    } else {
        currentState = STATE_NORMAL;
    }

    // Print debug values once every second
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime >= 1000) {
        lastDebugTime = millis();
        Serial.print("Current ADC value: ");
        Serial.println(currentAdc);
        
        Serial.print("Active fire zone: ");
        Serial.println(activeFireZone);
        
        Serial.print("Smoke detected: ");
        Serial.println(hasSmoke ? "YES" : "NO");
        
        Serial.print("High temperature: ");
        Serial.println(hasHighTemp ? "YES" : "NO");
        
        Serial.print("Motion detected: ");
        Serial.println(isMotionDetected ? "YES" : "NO");        
        Serial.print("Current system state: ");
        switch (currentState) {
            case STATE_NORMAL: 
                Serial.println("NORMAL"); 
                systemStatusStr = "NORMAL";
                alarmMessage = "";
                break;
            case STATE_HIGH_TEMP: 
                Serial.println("HIGH_TEMP"); 
                systemStatusStr = "HIGH TEMP";
                alarmMessage = "High Temp Detected!";
                break;
            case STATE_SMOKE: 
                Serial.println("SMOKE"); 
                systemStatusStr = "SMOKE";
                alarmMessage = "Smoke Detected!";
                break;
            case STATE_FIRE: 
                Serial.println("FIRE"); 
                systemStatusStr = "FIRE";
                alarmMessage = "Fire Detected!";
                break;
            case STATE_OVER_CURRENT: 
                Serial.println("OVER_CURRENT"); 
                systemStatusStr = "OVER CURRENT";
                alarmMessage = "Overcurrent Alarm!";
                break;
        }
        Serial.println("-------------------------");
    }

    // ==== EXECUTE ACTIONS ====
    
    // Buzzer Control (Priority to Over Current or Fire)
    if (currentState == STATE_OVER_CURRENT || currentState == STATE_FIRE) {
        digitalWrite(BUZZER_PIN, HIGH);
        isBuzzerOn = true;
    } else {
        digitalWrite(BUZZER_PIN, LOW);
        isBuzzerOn = false;
    }

    // Exhaust Fan Control (Smoke presence regardless of state priority display)
    if (hasSmoke) {
        digitalWrite(FAN_RELAY_PIN, LOW); // Relay ON (Active LOW)
        isFanOn = true;
    } else {
        digitalWrite(FAN_RELAY_PIN, HIGH); // Relay OFF (Active LOW)
        isFanOn = false;
    }

    // Servo Control and Pump (Fire localization)
    if (activeFireZone != -1) {
        bool zoneChanged = (currentActiveZone != activeFireZone);
        
        moveToZone(activeFireZone);
        
        if (isMotionDetected) {
            openDoor();
        } else {
            closeDoor();
        }
        
        if (zoneChanged) {
            delay(1500); // Allow time for servos to reach the position
        }
        
        pumpOn(); // Turn on the pump to spray water
    } else {
        closeDoor(); // Automatically close door if no active fire
        pumpOff();   // Ensure pump is off when there is no fire
    }
}

//========================= SERVO FUNCTIONS =========================
void moveToZone(int zone) {
    if (currentActiveZone == zone) {
        return; // Prevent continuous writing to servos to reduce jitter
    }
    
    currentActiveZone = zone;
    
    int bottomAngle = 0;
    int topAngle = 0;
    
    switch (zone) {
        case 1:
            bottomAngle = 0;
            topAngle = 0;
            break;
        case 4:
            bottomAngle = 90;
            topAngle = 0;
            break;
        case 3:
            bottomAngle = 180;
            topAngle = 0;
            break;
        case 2:
            bottomAngle = 180;
            topAngle = 90;
            break;
        default:
            return;
    }
    
    bottomServo.write(bottomAngle);
    topServo.write(topAngle);
}

bool openDoor() {
    if (!isDoorOpen) {
        doorServo.write(90);
        isDoorOpen = true;
        return true;
    }
    return false;
}

bool closeDoor() {
    if (isDoorOpen) {
        doorServo.write(0);
        isDoorOpen = false;
        return true;
    }
    return false;
}

//========================= PUMP FUNCTIONS =========================
void pumpOn() {
    digitalWrite(PUMP_RELAY_PIN, LOW); // Relay ON (Active LOW)
    isPumpOn = true;
}

void pumpOff() {
    digitalWrite(PUMP_RELAY_PIN, HIGH); // Relay OFF (Active LOW)
    isPumpOn = false;
}

//========================= BLYNK DATA SYNC =========================
void sendBlynkData() {
    // Zone 1
    Blynk.virtualWrite(V0, zones[0].temperature);
    Blynk.virtualWrite(V1, zones[0].smoke);
    Blynk.virtualWrite(V2, zones[0].flame);
    Blynk.virtualWrite(V3, zones[0].distance);

    // Zone 2
    Blynk.virtualWrite(V4, zones[1].temperature);
    Blynk.virtualWrite(V5, zones[1].smoke);
    Blynk.virtualWrite(V6, zones[1].flame);
    Blynk.virtualWrite(V7, zones[1].distance);

    // Zone 3
    Blynk.virtualWrite(V8, zones[2].temperature);
    Blynk.virtualWrite(V9, zones[2].smoke);
    Blynk.virtualWrite(V10, zones[2].flame);
    Blynk.virtualWrite(V11, zones[2].distance);

    // Zone 4
    Blynk.virtualWrite(V12, zones[3].temperature);
    Blynk.virtualWrite(V13, zones[3].smoke);
    Blynk.virtualWrite(V14, zones[3].flame);
    Blynk.virtualWrite(V15, zones[3].distance);

    // System States & Actuators
    Blynk.virtualWrite(V16, isPumpOn ? 1 : 0);
    Blynk.virtualWrite(V17, isMotionDetected ? 1 : 0);
    Blynk.virtualWrite(V18, currentState == STATE_OVER_CURRENT ? 1 : 0);
    
    // Servo Angle (Bottom Servo representation)
    int angle = 0;
    if (currentActiveZone == 1) angle = 0;
    else if (currentActiveZone == 4) angle = 90;
    else if (currentActiveZone == 3) angle = 180;
    else if (currentActiveZone == 2) angle = 180; 
    Blynk.virtualWrite(V19, angle);

    Blynk.virtualWrite(V20, isFanOn ? 1 : 0);
    Blynk.virtualWrite(V21, isDoorOpen ? 1 : 0);
    Blynk.virtualWrite(V22, systemStatusStr);
    Blynk.virtualWrite(V23, activeFireZone == -1 ? 0 : activeFireZone);
    Blynk.virtualWrite(V24, alarmMessage);
    Blynk.virtualWrite(V25, isBuzzerOn ? 1 : 0);
}
