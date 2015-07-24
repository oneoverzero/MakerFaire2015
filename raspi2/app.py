from threading import Thread
from vars import *
from serial1 import *
from serial2 import *

import time


# main starts here
logging.debug('and so it begins!')

# setup and start Serial1 thread
ser1 = Serial1(serialPort1, serialBaud1)

# setup and start Serial1 thread
ser2 = Serial2(serialPort2, serialBaud2)



time.sleep(2)
ser1.kill()
ser2.kill()


logging.debug('and it ends')

