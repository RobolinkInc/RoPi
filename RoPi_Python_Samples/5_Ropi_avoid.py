import RoPi_SerialCom as ropi
import time

#grab the time of the start of the program
startTime = time.time()

#lets set the speed to 80%
ropi.speed(50)

#run this loop while the time elapsed is under 15 seconds
while((time.time()-startTime) < 15):

    #read the top IR sensors
    leftIR, middleIR, rightIR = ropi.readTopIRsensors()

    #detect something on the left move right
    if leftIR < 200:
        ropi.moveRight()
        print("right")
        time.sleep(0.2)
    #detect something on the right move left    
    elif rightIR < 200:
        ropi.moveLeft()
        print("left")
        time.sleep(0.2)
    #detect something ahead, move backward    
    elif middleIR < 200:
        print("Backward")
        ropi.moveBackwards()
        time.sleep(1)
        ropi.moveLeft()
        time.sleep(0.2)
    #nothing ahead move straight ahead
    else:
        print("Forward")
        ropi.moveForwards()
        

#make sure to stop the robot at the very end        
ropi.moveStop()
print("Program done!")
