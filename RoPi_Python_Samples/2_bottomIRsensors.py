import RoPi_SerialCom as ropi

#create a loop so we can keep reading the top IR sensors
while(1):
    leftBottomIR, middleBottomIR, rightBottomIR = ropi.readBottomIRSensors()
    print(leftBottomIR, middleBottomIR, rightBottomIR)
    #this will print the information
    #you can test it this by placing your fingers
    #on the bottom of the Smart Inventor board
    #the values returned are either 0 or 1.
    #1 for covered and 0 for not covered
