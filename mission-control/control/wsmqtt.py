import cherrypy
import paho.mqtt.publish as paho

class WS(object):
  @cherrypy.expose
  def index(self,msg=""):
    print "got this! " + msg
    if msg[:3] == 'MIS' and msg[-3:] == 'SIM':
      paho.single("rover/command",msg,hostname='127.0.0.1')
    return "ok"

if __name__ == '__main__':
    cherrypy.quickstart(WS())
