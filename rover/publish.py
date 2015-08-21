import paho.mqtt.publish as paho
from vars import *

paho.single("rover/telemetry","Lorem ipsum dolor sit amet",hostname=mqttServer)

