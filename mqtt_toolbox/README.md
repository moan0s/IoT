# Create environment


Create an environment file `.env` with your broker details
```env
MQTT_BROKER_USER=username
MQTT_BROKER_PASSWORD="super-secret"
```

# Start MQTT client

The client produces artificial temperature readings that will be published to
the topic `sensors/temperature` and `sensors/temperature2`

```bash
python fake_client.py
```

# Subscribe to the topic

To test if the values reach the broker and are accessible try to subscribe to
the topic sensors

```bash
python subscribe.py
```
