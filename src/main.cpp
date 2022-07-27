#include <Arduino.h>

#include <ArduinoJson.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Count Trash
const int trigPin1 = 8; 
const int echoPin1 = 7;
unsigned long distance1;
long duration1;

// Trash Level
const int trigPin2 = 2;
const int echoPin2 = 3;
unsigned long distance2;
long duration2;

int trash=0;
float trashLevel=0;

long currentTime;
long prevTime = 0;
long prevTime2 = 0;
int mqttInterval=5000; //Interval sending data to MQTT
int trashInterval=1000; //Interval trashing time

byte mac[] = {0x12, 0x3A, 0x00, 0xBB, 0x00, 0x09}; //hex
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
// const char *mqttServer = "192.168.1.5";
// const int mqttPort = 1883;
// const char *mqttUser = "";
// const char *mqttPass = "";
// const char *mqttTopic = "mqtt/d-sensor";

const char *mqttServer = "mqtt-io.kku.ac.th";
const int mqttPort = 1991;
const char *mqttUser = "trash";
const char *mqttPass = "1q2w3e4r@trash";
const char *mqttTopic = "mqtt/d-sensor";
char *mqttPayload;

int id = 2;

void print_ip_details()
{
  Serial.print("Ethernet Status : ");
  Serial.println(Ethernet.linkStatus());
  Serial.print("IP Address : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("IP Gateway : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS : ");
  Serial.println(Ethernet.dnsServerIP());
}

void initEthernet()
{
  Serial.println("Ethernet initiation, Just wait a minute.");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Don't receive ip from dhcp server.");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield connection error!");
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Cable is not connected.");
    }
    else
    {
      Serial.println("Error!");
    }
    while (true)
    {
      delay(200);
    }
  }
  if (Ethernet.linkStatus() == 0)
  {
    print_ip_details();
  }
  else
  {
    Serial.println("Error!");
  }
}

void read1() //trashing
{
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = (duration1 /29.1)/2;
  Serial.print("Distance1: ");
  Serial.print(distance1);
  Serial.print(" cm. ");

  if (distance1 < 45 && (currentTime - prevTime2 >= trashInterval))
  {
    prevTime2 = currentTime;
    trash++;
  }
  Serial.print("Trash: ");
  Serial.print(trash);
  Serial.print(" time(s). ");
}

void read2() //trash level
{
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = (duration2/29.1) / 2;
  Serial.print("Distance2: ");
  Serial.print(distance2);
  Serial.print(" cm. ");

  trashLevel = map(distance2,0,158,100,0);   //(max-x)/(max-min)*100
  Serial.print("Trash Level: ");
  Serial.print(trashLevel);
  Serial.println(" %. ");
}


void setup()
{
  Serial.begin(921000);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  initEthernet();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.connect("arduinoClient", mqttUser, mqttPass);
}

void sendToMqtt()
{
  if (currentTime - prevTime >= mqttInterval)
  {
    prevTime = currentTime;
    String count_trash = String(trash);
    String count_level = String(trashLevel);
    String jData = String("{\"id\": ") + id + String(",\"trash\": ") + count_trash + String(", \"level\": ") + count_level + String("}");
    char buf[100];
    jData.toCharArray(buf, 100);
    mqttPayload = buf;
    mqttClient.publish(mqttTopic, mqttPayload);
    if (!mqttClient.connected())
    {
      mqttClient.connect("arduinoClient", mqttUser, mqttPass);
    }
  }
  
  
}
void loop()
{
  currentTime = millis();
  read1();
  delay(100);
  read2();
  delay(100);
  sendToMqtt();
}

