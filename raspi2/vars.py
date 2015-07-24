#
# we import stuff globally here
#
import logging
import serial

#
#define global variables here
#
mqttServer  = "localhost"
serialPort1 = "/dev/ttyACM0"
serialBaud1 = 19200 
serialPort2 = "/dev/ttyUSB0"
serialBaud2 = 19200 


logging.basicConfig(level=logging.DEBUG,format='(%(threadName)-10s) %(message)s',)

