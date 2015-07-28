import paho.mqtt.publish as paho
from vars import *

paho.single("rover/command","MIS|FWD|FWD|LEF|FWD|FWD|RIG|FWD|FWD|SIM",hostname=mqttServer)

