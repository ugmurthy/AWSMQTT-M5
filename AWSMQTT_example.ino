// Demonstrate use of M5 Stick as an IOT device communicating with AWS IOT using MQTT
// 1. Compile this code after changing secrets.h (instructions within secrets.h)
// 2. Setup two THINGS on AWS IOT (see getting started documentation on AWS)
// 3. Ensure right policies are attached to the certificates of the THING
// 4. if all goes well - you should be able to start and stop data coming from M5-Stick by publishing an m5/on or m5/off topic
// 5. using m5/status you can fetch the M5 board status in terms of battery volt/current, and internal temperature
// 
// 25-Dec-2020
// uses library in the foloder "arduino-mqtt"
// downloaded as zip file from
// https://github.com/256dpi/arduino-mqtt

// Part of the code is from :
// https://aws.amazon.com/blogs/compute/building-an-aws-iot-core-device-using-aws-serverless-and-an-esp32/
//
// 25/Dec/2021
// Code added by Murthy ugmurthy (at) gmail.com
// Relates to the MLU - reading accellerometer data
// the MQTT interactions to seek some board related readings (battery,usb - voltage/current, temp etc)
// overall integration
//
#include <M5StickCPlus.h>

#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// power managment - m5/status will return these values 
#include "I2C_AXP192.h"
I2C_AXP192 axp192(I2C_AXP192_DEFAULT_ADDRESS, Wire1);
const int btnPin = 37;

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "m5/read"
#define AWS_IOT_SUBSCRIBE_TOPIC "m5/#"
#define M5_ON "m5/on"
#define M5_OFF "m5/off"
#define M5_ACC "m5/read"
#define M5_STATUS "m5/status"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

void clearscreen();

void clearscreen() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE ,BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
}



void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  M5.Lcd.println("Connecting to Wi-Fi");
  Serial.println("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("CONNECTED to Wi-Fi");
  M5.Lcd.println("Connected to Wi-Fi");
  Serial.println("Configuring Client Secure to use AWS IOT");
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  Serial.println("Connecting to MQTT Broker :");
  Serial.println(AWS_IOT_ENDPOINT);
  
  // Create a message handler
  client.onMessage(messageHandler);
  
  Serial.print("Connecting to AWS IOT THINGNAME : ");
  Serial.print(THINGNAME);
  Serial.println();
  
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nChecking connection");
  
  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(M5_ON);
  client.subscribe(M5_OFF);
  client.subscribe(M5_ACC);
  client.subscribe(M5_STATUS);
  
  M5.Lcd.println("AWS IoT Connected!");
  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  // 200 is the RAM allocated to this document
  StaticJsonDocument<200> doc;
  // add key, values to doc
  doc["time"] = millis();
  //doc["sensor_a0"] = analogRead(0);
  doc["accX"] = accX;
  doc["accY"] = accY;
  doc["accZ"] = accZ;
  
  char jsonBuffer[512];
  // convert to text
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(String &topic, String &payload) {
  StaticJsonDocument<200> msgdoc;
  
  //Serial.println("incoming: " + topic + " - " + payload);

  // Are just getting Accelerometer readings?
  if (topic == M5_ACC) {
      Serial.println("ACC Readings : " + payload);
  } else {
      // check if we are to put it on
      if (topic == M5_ON ) {
          send_readings = 1;
          Serial.print("ON/OFF? : " + topic + " send_readings is now " ); 
          Serial.println(send_readings);
      }
      // check if we are to put it off
      if (topic == M5_OFF) {
          send_readings = 0;
          Serial.print("ON/OFF? : " + topic + " send_readings is now " ); 
          Serial.println(send_readings);
      }  
      if (topic == M5_STATUS) {
        getStatus();
      }
      
  }

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void getStatus() {
  StaticJsonDocument<200> status_doc;
  char jsonBuffer[512];
  
  float b_volt = axp192.getBatteryVoltage();
  float b_discharge_i = axp192.getBatteryDischargeCurrent();
  float b_charge_i =axp192.getBatteryChargeCurrent();
  float bus_volt = axp192.getVbusVoltage();
  float bus_i = axp192.getVbusCurrent();
  float i_temp = axp192.getInternalTemperature();
  

  status_doc["battery_voltage"]=b_volt;
  status_doc["battery_discharge_current"]=b_discharge_i;
  status_doc["battery_charge_current"]=b_charge_i;
  status_doc["bus_voltage"]=bus_volt;
  status_doc["bus_current"]=bus_i;
  status_doc["internal_temperature"]=i_temp;
  status_doc["interval"]=interval;
  status_doc["send_readings"]=send_readings;
  
  // convert to text
  serializeJson(status_doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.print("Status : ");
  Serial.println(jsonBuffer);
  }

void setup() {
  Serial.begin(9600);
  M5.begin();
  M5.Lcd.setRotation(3);
  clearscreen();
  connectAWS();
  //Initialise MPU6886
  M5.Imu.Init();
  
  // Power management
  Wire1.begin(21, 22);
  I2C_AXP192_InitDef initDef = {
    .EXTEN  = true,
    .BACKUP = true,
    .DCDC1  = 3300,
    .DCDC2  = 0,
    .DCDC3  = 0,
    .LDO2   = 3000,
    .LDO3   = 3000,
    .GPIO0  = 2800,
    .GPIO1  = -1,
    .GPIO2  = -1,
    .GPIO3  = -1,
    .GPIO4  = -1,
  };
  axp192.begin(initDef);
  pinMode(btnPin, INPUT);
}

void loop() {

  if (digitalRead(btnPin) == 0) {
    Serial.printf("powerOff()\n");
    Serial.flush();
    delay(1000);
    axp192.powerOff();
  }
  
  // send_readings will be switch on by a topic - m5/on and off by m5/off
  if (send_readings) {
    M5.Lcd.setCursor(15,90);
    M5.Lcd.println(" Acc is ON ");
    M5.Imu.getAccelData(&accX, &accY, &accZ);
    publishMessage();
  } else {
    M5.Lcd.setCursor(15,90);
    M5.Lcd.println(" ACC is OFF");  
  }
  client.loop();
  delay(interval);
}
