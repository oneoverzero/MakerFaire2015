import paho.mqtt.publish as paho
from vars import *

import socket
import fcntl
import struct

def get_ip_address(ifname):
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  return socket.inet_ntoa(fcntl.ioctl(s.fileno(),0x8915,  # SIOCGIFADDR
    struct.pack('256s', ifname[:15]))[20:24])

mqttServer = "192.168.69.3"

paho.single("rover/ip",get_ip_address("wlan0"),hostname=mqttServer)

