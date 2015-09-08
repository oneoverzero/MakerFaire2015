import paho.mqtt.publish as paho
from vars import *

paho.single("rover/command","MIS22334455SIM",hostname=mqttServer)

