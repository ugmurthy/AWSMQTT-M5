# AWSMQTT-M5
Accelerometer data from M5 Stick C plus over MQTT to AWS IOT with start/stop control

The key purpose of this code is to demonstrate communicating between a M5-Stick and AWS-IOT. In this demonstration
the following MQTT topics are used:

* M5/on (start sending accelerometer data)
* M5/off (stop sending accelerometer data)
* M5/status (read  M5-stick device parameters )
* M5/read (read accelerometer readings)


Assumes familiarity with AWS IOT and Arduino IDE
