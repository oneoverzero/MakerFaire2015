import string
import random
import puny

baseURI = 'http://cdn.labs.oneoverzero.org/MakerFaire2015/'

def get_hash():

  def id_generator(size=4, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))

  return id_generator()+'-'+id_generator()


print "id,shortURI,URI,command"


for a in range(512):

  randhash = get_hash()
  
  uri = baseURI + randhash + '/'

  print str(a) + ',"' + puny.shorten(uri).ascii + '","' + uri + '","mkdir ' + randhash + '"'



