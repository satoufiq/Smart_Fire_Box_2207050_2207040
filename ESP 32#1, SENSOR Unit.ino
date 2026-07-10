#include <DHT.h>

#define DHTTYPE DHT11

//========================= ZONE 1 =========================

#define DHT1_PIN     17
#define TRIG1_PIN    4
#define ECHO1_PIN    16
#define MQ1_PIN      32
#define FLAME1_PIN   5

//========================= ZONE 2 =========================

#define DHT2_PIN     18
#define TRIG2_PIN    19
#define ECHO2_PIN    21
#define MQ2_PIN      39
#define FLAME2_PIN   23

//========================= ZONE 3 =========================

#define DHT3_PIN     27
#define TRIG3_PIN    26
#define ECHO3_PIN    25
#define MQ3_PIN      34
#define FLAME3_PIN   22

//========================= ZONE 4 =========================

#define DHT4_PIN     33
#define TRIG4_PIN    14
#define ECHO4_PIN    13
#define MQ4_PIN      35
#define FLAME4_PIN   15

//========================= DHT OBJECTS =========================

DHT dht1(DHT1_PIN, DHTTYPE);
DHT dht2(DHT2_PIN, DHTTYPE);
DHT dht3(DHT3_PIN, DHTTYPE);
DHT dht4(DHT4_PIN, DHTTYPE);

//========================= VARIABLES =========================

float temp1,temp2,temp3,temp4;

int smoke1,smoke2,smoke3,smoke4;

bool flame1,flame2,flame3,flame4;

long dist1,dist2,dist3,dist4;

String packet;

//========================= SONAR FUNCTION =========================

long getDistance(int trigPin,int echoPin)
{
    digitalWrite(trigPin,LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin,HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin,LOW);

    long duration = pulseIn(echoPin,HIGH,30000);

    if(duration==0)
        return -1;

    return duration * 0.0343 / 2;
}

//========================= SETUP =========================

void setup()
{
    // UART0
    Serial.begin(9600);

    dht1.begin();
    dht2.begin();
    dht3.begin();
    dht4.begin();

    pinMode(TRIG1_PIN,OUTPUT);
    pinMode(TRIG2_PIN,OUTPUT);
    pinMode(TRIG3_PIN,OUTPUT);
    pinMode(TRIG4_PIN,OUTPUT);

    pinMode(ECHO1_PIN,INPUT);
    pinMode(ECHO2_PIN,INPUT);
    pinMode(ECHO3_PIN,INPUT);
    pinMode(ECHO4_PIN,INPUT);

    pinMode(FLAME1_PIN,INPUT);
    pinMode(FLAME2_PIN,INPUT);
    pinMode(FLAME3_PIN,INPUT);
    pinMode(FLAME4_PIN,INPUT);

    delay(2000);
}

void loop()
{
        //==================== READ DHT11 ====================

    temp1 = dht1.readTemperature();
    temp2 = dht2.readTemperature();
    temp3 = dht3.readTemperature();
    temp4 = dht4.readTemperature();

    //==================== READ MQ2 ====================

    smoke1 = analogRead(MQ1_PIN);
    smoke2 = analogRead(MQ2_PIN);
    smoke3 = analogRead(MQ3_PIN);
    smoke4 = analogRead(MQ4_PIN);

    //==================== READ FLAME ====================

    flame1 = !digitalRead(FLAME1_PIN);
    flame2 = !digitalRead(FLAME2_PIN);
    flame3 = !digitalRead(FLAME3_PIN);
    flame4 = !digitalRead(FLAME4_PIN);

    //==================== READ SONAR ====================

    dist1 = getDistance(TRIG1_PIN,ECHO1_PIN);
    dist2 = getDistance(TRIG2_PIN,ECHO2_PIN);
    dist3 = getDistance(TRIG3_PIN,ECHO3_PIN);
    dist4 = getDistance(TRIG4_PIN,ECHO4_PIN);
        //==================== BUILD UART PACKET ====================

    packet = "";

    // Zone 1
    packet += "Z1,";
    packet += String(flame1);
    packet += ",";
    packet += String(smoke1);
    packet += ",";
    packet += String(temp1,1);
    packet += ",";
    packet += String(dist1);
    packet += ";";

    // Zone 2
    packet += "Z2,";
    packet += String(flame2);
    packet += ",";
    packet += String(smoke2);
    packet += ",";
    packet += String(temp2,1);
    packet += ",";
    packet += String(dist2);
    packet += ";";

    // Zone 3
    packet += "Z3,";
    packet += String(flame3);
    packet += ",";
    packet += String(smoke3);
    packet += ",";
    packet += String(temp3,1);
    packet += ",";
    packet += String(dist3);
    packet += ";";

    // Zone 4
    packet += "Z4,";
    packet += String(flame4);
    packet += ",";
    packet += String(smoke4);
    packet += ",";
    packet += String(temp4,1);
    packet += ",";
    packet += String(dist4);
    packet += "#";
        //==================== SEND TO ESP32 #2 ====================

    Serial.println(packet);

    delay(1000);

}