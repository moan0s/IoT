import time
import paho.mqtt.client as mqtt
import numpy
import numpy as np
import os
from dotenv import load_dotenv

load_dotenv()

def calc_temp(amplitude=5, mean=20, offset=0, period=900):
    """
    amplitude: amplitude of temperature changes
    mean: mean of the simulated temperature
    offset: offset of the signal in seconds
    period: periond time in seconds
    """
    temp = np.sin((time.time()+offset)%(period)*2*np.pi)*amplitude+mean
    return temp

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    if (rc==5):
        raise ConnectionError("MQTT server refused connection")

client = mqtt.Client()
client.username_pw_set(username=os.getenv("MQTT_BROKER_USER"),
                       password=os.getenv("MQTT_BROKER_PASSWORD"))
client.on_connect = on_connect

client.connect(os.getenv("MQTT_BROKER_HOSTNAME"), 1883, 60)

client.loop_start()

while True:
    time.sleep(2)
    client.publish("sensors/temperature", calc_temp())
    client.publish("sensors/temperature2", calc_temp(offset=180))

