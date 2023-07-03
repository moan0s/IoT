#!/usr/bin/env python
import paho.mqtt.client as mqtt
import os
import random
from dotenv import load_dotenv

load_dotenv()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("sensors/analog_raw")
    client.subscribe("sensors/temperature")
    client.subscribe("sensors/temperature2")
    client.subscribe("sensors/fake_temperature2")

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))
client_id = f'python-mqtt-{random.randint(0, 1000)}'
client = mqtt.Client(client_id)
client.username_pw_set(username=os.getenv("MQTT_BROKER_USER"),
                       password=os.getenv("MQTT_BROKER_PASSWORD"))
client.on_connect = on_connect
client.on_message = on_message

client.connect(os.getenv("MQTT_BROKER_HOSTNAME"), 1883, 60)

client.loop_forever()

