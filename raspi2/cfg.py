import paho.mqtt.publish as paho
from vars import *

paho.single("rover/command","CFGEGFC",hostname=mqttServer)

