#
# we import stuff globally here
#
import logging
import serial

#
#define global variables here
#
mqttServer  = "10.42.0.1"

serialPort1 = "/dev/ttyAMA0"
serialBaud1 = 19200 
serialPort2 = "/dev/ttyACM0"
serialBaud2 = 19200 

logging.basicConfig(level=logging.DEBUG,format='(%(threadName)-10s) %(message)s',)

