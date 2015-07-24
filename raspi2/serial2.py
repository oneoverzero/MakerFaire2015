# Serial2 Class
from threading import Thread
from vars import *
import paho.mqtt.client as paho

class Serial2(Thread):

  def __init__(self,serialPort2,serialBaud2):
    Thread.__init__(self)
    self.daemon = True
    self.running = True
    self.name = 'Serial 2'
    self.start()

  def kill(self): 
    self.running = False
      
  #define what happens after connection
  def on_connect(self,nil,pahoClient,obj,rc):
    logging.debug('MQTT client connected, status ' + str(rc))
    self.client.subscribe("rover/telemetry")

  #On receipt of a message create a pynotification and show it
  def on_message(self,pahoClient,obj,msg):
    if msg.payload[:3] == "BEG" :
      logging.debug('got telemetry data -> ' + msg.payload)
      
  def run(self):
    logging.debug('MQTT client starting')
    self.client = paho.Client()
    self.client.on_connect = self.on_connect
    self.client.on_message = self.on_message
    self.client.connect(mqttServer,1883,60)
    while True:
      if not self.running:
        break
      self.client.loop()

        
        
