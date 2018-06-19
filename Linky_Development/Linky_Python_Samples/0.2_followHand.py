import RoPi_SerialCom as ropi
import time

time.sleep(2)

topLeft,topMiddle,topRight,bottomLeft,bottomRight,batterylvl, miclvl = ropi.readSensors()

maxSpeed = int(batterylvl*0.2381*100)
#calculate the max speed the motors can go at
#batt can reach 4.20 volts which will result in 100% motor speed

#we now start the robot at 50% the max speed
ropi.speed(int(maxSpeed*0.5))
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

