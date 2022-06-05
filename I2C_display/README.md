Zhis is aimed to us a I2C display eith a raspberry pi

# Usage

- Activate I2C via `raspi-config`
- Install Adafruit library Adafruit Python SSD1306 
- Connect display (e.g. 0,96'' OLED 128\*64px)
- execute example with `python3 example.py`

# Example: Show IP address of pi at start

Add the file `/etc/systemd/system/stats.service` with thevfollowing content

```

[Unit]
Description=Show stats on I2C display
After=network.target

[Service]
ExecStart=python3 /home/pi/Adafruit_Python_SSD1306/examples/stats.py

[Install]
WantedBy=multi-user.target
```
