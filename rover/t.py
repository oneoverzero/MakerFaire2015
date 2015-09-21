import paho.mqtt.publish as paho
from vars import *

paho.single("rover/command","MIS24242424242SIM",hostname=mqttServer)

