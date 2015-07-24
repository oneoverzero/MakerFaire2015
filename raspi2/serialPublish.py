#
# this code will run in the Raspi2
# it gets data off Arduino1 (telemetry)
# and posts it into mission control rover/telemetry
#
import paho.mqtt.publish as paho
import serial
from vars import *

# needs adjustment for internal serial port
port = serial.Serial(serialPort1, serialBaud)

#
# processes a whole line holding telemetry
# typical string data is something like "BEG|00|01|02|03|04|05|END"
#
def readlineCR(port):
  buf = ''
  while True:
    char = port.read()
    if char == 'B':
      buf = char
    else:
      buf += char
      if char=='\r' or char=='':
      // TODO add verification to start and end text
        return buf

#
# loops 4evar, sending data
#
while True:
  data = readlineCR(port)
  paho.single("rover/telemetry",data,hostname=mqttServer)
  
