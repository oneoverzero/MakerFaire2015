import cherrypy
import paho.mqtt.publish as paho
from vars import *

class WS(object):
  @cherrypy.expose
  def index(self,msg=""):
    print "got this! " + msg
    # if message is correct, publish command into MQTT
    if msg[:3] == 'MIS' and msg[-3:] == 'SIM':
      paho.single("rover/command",msg,hostname=mqttServer)
    return "ok"

if __name__ == '__main__':
    cherrypy.quickstart(WS())
