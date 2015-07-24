#
# this code will run in the Raspi2
# it gets data off Arduino1 (telemetry)
# and posts it into mission control rover/telemetry
#
import paho.mqtt.publish as paho
import serial
from vars import *

def on_publish(mosq, userdata, mid):
  # Disconnect after our message has been sent.
  mosq.disconnect()

# Specifying a client id here could lead to collisions if you have multiple
# clients sending. Either generate a random id, or use:
#client = mosquitto.Mosquitto()
client = mosquitto.Mosquitto("image-send")
client.on_publish = on_publish
client.connect("127.0.0.1")

f = open("data")
imagestring = f.read()
byteArray = bytes(imagestring)
client.publish("photo", byteArray ,0)
# If the image is large, just calling publish() won't guarantee that all 
# of the message is sent. You should call one of the mosquitto.loop*()
# functions to ensure that happens. loop_forever() does this for you in a
# blocking call. It will automatically reconnect if disconnected by accident
# and will return after we call disconnect() above.
client.loop_forever()

