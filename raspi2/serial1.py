# Serial1 Class
from threading import Thread
from vars import *
import paho.mqtt.publish as mqttp

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

  def find_collision(self,data):
    part = data.split('|')

    if (part[1] != "0.00" ):
	  return "100"

    if (part[2] != "0.00" ):
	  return "010"

    if (part[3] != "0.00" ):
	  return "001"

    return "000"

  def run(self):
    try:
      self.port = serial.Serial(serialPort1, serialBaud1)
      logging.debug('starting')
    except:
      pass  
      
    while True:
      try:
        data = self.readline()
        logging.debug(data)
        collision = find_collision(data)
        if (collision != "000" ):
          mqttp.single("rover/command","COL" + collision + "LOC",hostname=mqttServer)

        mqttp.single("rover/telemetry",data,hostname=mqttServer)
      except:
        pass
        
      if not self.running:
        break
        
