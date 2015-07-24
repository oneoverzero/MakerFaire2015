# Serial1 Class
from threading import Thread
from vars import *
import paho.mqtt.publish as paho

class Serial1(Thread):

  def __init__(self,serialPort1,serialBaud1):
    Thread.__init__(self)
    self.daemon = True
    self.running = True
    self.name = 'Serial 1'
    self.start()

  def kill(self): 
    self.running = False
    
  def readline(self):
    buf = ''
    while True:
      char = self.port.read()
      if char == 'B':
        buf = char
      else:
        buf += char
        
      if char=='\r' or char=='' and buf[:3] == 'BEG' and buf[-4:] == 'END\r':
          return buf

  def run(self):
    self.port = serial.Serial(serialPort1, serialBaud1)
    logging.debug('starting')
    while True:
      data = self.readline()
      logging.debug(data)
      paho.single("rover/telemetry",data,hostname=mqttServer)
      if not self.running:
        break
        
