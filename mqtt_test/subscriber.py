#!/usr/bin/env python
import paho.mqtt.client as mqtt
import os
from dotenv import load_dotenv

load_dotenv()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("sensors/#")

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
client.username_pw_set(username=os.getenv("MQTT_BROKER_USER"),
                       password=os.getenv("MQTT_BROKER_PASSWORD"))
client.on_connect = on_connect
client.on_message = on_message

client.connect(os.getenv("MQTT_BROKER_HOSTNAME"), 1883, 60)

client.loop_forever()

