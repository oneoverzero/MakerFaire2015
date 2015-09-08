# Serial2 Class
from threading import Thread
from vars import *
import paho.mqtt.client as mqttc

class Serial2(Thread):

  def __init__(self,serialPort2,serialBaud2):
    Thread.__init__(self)
    self.daemon = True
    self.running = True
    self.name = 'Serial 2'
    self.cmdList = [0,1,2,3,4,5,6,7]
    self.start()

  def kill(self): 
    self.running = False
      
  #define what happens after connection
  def on_connect(self,nil,pahoClient,obj,rc):
    logging.debug('MQTT client connected, status ' + str(rc))
    self.client.subscribe("rover/command")

  #On receipt of a message fill cmdList array with commands
  def on_message(self,pahoClient,obj,msg):
    payload = msg.payload
    if payload[:3] == 'MIS' and payload[-3:] == 'SIM':
      logging.debug('got Mission Control data -> ' + payload)
      try:
        self.port.write(payload+'\n')
        logging.debug('sent to Arduino2')
      except:
        pass
      
  def run(self):
    logging.debug('MQTT client starting')
    self.client = mqttc.Client()
    self.client.on_connect = self.on_connect
    self.client.on_message = self.on_message
    self.client.connect(mqttServer,1883,60)
    try:
      self.port = serial.Serial(serialPort2, serialBaud2)
      logging.debug('starting')
    except:
      pass  

    while True:
      if not self.running:
        break
      self.client.loop()

        
        
