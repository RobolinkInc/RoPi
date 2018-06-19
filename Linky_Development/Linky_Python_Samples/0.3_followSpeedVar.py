import RoPi_SerialCom as ropi
import time

time.sleep(2)

while (1):
    topLeft,topMiddle,topRight,bottomLeft,bottomRight,batterylvl, miclvl = ropi.readSensors()
    print("IR:",topLeft,topMiddle,topRight,bottomLeft,bottomRight,"battery",batterylvl,"mic level", miclvl)
    
    if topLeft < 200:
        ropi.right()
    elif topRight < 200:
        ropi.left()
    elif topMiddle < 200:
        ropi.forward()
    else:
        ropi.stop()

