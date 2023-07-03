import argparse
import paho.mqtt.client as mqtt
import random
import numpy as np
import time


def gen_time():
    return time.strftime("%H:%M:%S", time.localtime())


def gen_temp():
    """
    amplitude: amplitude of temperature changes
    mean: mean of the simulated temperature
    offset: offset of the signal in seconds
    period: periond time in seconds
    """
    amplitude = 5
    mean = 20
    offset = 0
    period = 900
    temp = np.sin((time.time() + offset) % (period) * 2 * np.pi) * amplitude + mean
    return temp


def fake(client, data_type="temperature", topic="sensors/temperature"):
    if data_type == "temperature":
        data_generator = gen_temp
    elif data_type == "time":
        data_generator = gen_time

    while True:
        print("Publishing")
        if data_type == "temperature":
            client.publish(topic, gen_temp())
        elif data_type == "time":
            client.publish(topic, gen_time())
        time.sleep(2)


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    if rc == 5:
        raise ConnectionError("MQTT server refused connection")


def connect_broker(server, port, username="", password=""):
    client_id = f'python-mqtt-{random.randint(0, 1000)}'
    client = mqtt.Client(client_id)
    client.on_connect = on_connect
    client.username_pw_set(username=username,
                           password=password)
    client.connect(server, port, 60)

    return client


def cli():
    parser = argparse.ArgumentParser(description='Do basic MQTT operations')
    parser.add_argument('action', choices=['sub', 'pub', 'fake'], help="")
    parser.add_argument('-t', '--topic', help="The MQTT topic")
    parser.add_argument('--type', help="The type of data to fake")
    parser.add_argument('-b', '--broker', help="Hostname of the MQTT broker")
    parser.add_argument('--port', type=int, default=1883, help="The MQTT brokers port (default: 1883)")
    parser.add_argument('-u', '--user', help="User for the MQTT broker")
    parser.add_argument('-p', '--password', help="Password of the MQTT broker")
    parser.add_argument('-d', '--payload', help="The payload to send with a set command")

    args = parser.parse_args()

    client = connect_broker(args.broker, args.port, args.user, args.password)
    if args.action == "sub":
        client.subscribe(args.topic)
        client.on_message = on_message
        client.loop_forever()
    elif args.action == "pub":
        r = client.publish(args.topic, payload=args.payload)
        print(r)
    elif args.action == "fake":
        fake(client, args.topic)


if __name__ == "__main__":
    cli()
