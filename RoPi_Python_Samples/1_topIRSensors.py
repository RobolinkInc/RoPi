import RoPi_SerialCom as ropi

#create a while loop so we can read the top IR sensors
while(1):
    leftIR, middleIR, rightIR = ropi.readTopIRsensors()
    print(leftIR, middleIR, rightIR)
    #this will print the information
    #you can test it this by placing your fingers
    #in front of the smart inventor board

    #the info it is showing is a number between 0-1023
    #notice what happens with the number as you bring your
    #hand closer to the sensor
