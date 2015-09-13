import cv2
import urllib
import numpy as np
 
server = "10.42.0.30"
server = "192.168.69.89"

stream=urllib.urlopen('http://'+server+':8080/?action=stream')
bytes=''
while True:
  bytes+=stream.read(1024)
  a = bytes.find('\xff\xd8')
  b = bytes.find('\xff\xd9')
  if a!=-1 and b!=-1:
    jpg = bytes[a:b+2]
    bytes= bytes[b+2:]
    i = cv2.imdecode(np.fromstring(jpg, dtype=np.uint8),cv2.CV_LOAD_IMAGE_COLOR)
    #cv2.imshow('i',i)
    cv2.imwrite('imagem.jpg',i)
    if cv2.waitKey(1) == 27:
      exit(0)

