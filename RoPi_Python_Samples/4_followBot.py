#this robot will try to follow an object in front of it
#this program will run the robot for 15 seconds

import RoPi_SerialCom as ropi
import time

#grab the time of the start of the program
startTime = time.time()

#lets set the speed to 80%
ropi.speed(80)

#run this loop while the time elapsed is under 15 seconds
while((time.time()-startTime) < 15):

    #read the top IR sensors
    leftIR, middleIR, rightIR = ropi.readTopIRsensors()
    

    if leftIR < 200:
        ropi.moveLeft()
        time.sleep(0.2)
        print("left")
        
    elif rightIR < 200:
        ropi.moveRight()
        time.sleep(0.2)
        print("right")

    elif middleIR < 200:
        print("Forwards")
        ropi.moveForwards()
        time.sleep(0.2)

    else:
        print("Nothing Detected")
        ropi.moveStop()

#make sure to stop the robot at the end of the 15 seconds       
ropi.moveStop()
print("Program done!")
