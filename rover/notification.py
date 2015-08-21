import pynotify
import paho.mqtt.client as paho
from vars import *

#define what happens after connection
def on_connect(pahoClient,obj,rc):
    print "Connected, status " + str(rc)
    client.subscribe("rover/telemetry")

#On receipt of a message create a pynotification and show it
def on_message(pahoClient,obj,msg):
  if msg.payload[:3] == "BEG" :
    print "got telemetry data -> " + msg.payload
  elif msg.payload[:3] != "BEG":
    pynotify.init("MQTT")
    n = pynotify.Notification (msg.topic, msg.payload, "/usr/share/pixmaps/nohost.png")
    n.show()
    print "got notification from " + msg.topic + "."
    #print "payload " + msg.payload + "."

client = paho.Client()
client.on_connect = on_connect
client.on_message = on_message

x = client.connect(mqttServer,1883,60)
client.loop_forever()
