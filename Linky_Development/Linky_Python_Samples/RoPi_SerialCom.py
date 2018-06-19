#this program reads the serial port
#at a baud rate of 115200
#This code works with Linky

import serial

ser = serial.Serial('/dev/ttyUSB0',115200)


def readSensors():
    #FrontLeft,FrontMiddle,FrontRight
    #SBottomLeft,SBottomRight
    #Battery Level
    #Mic level
    ser.write("L")
    read_serial = ser.readline()
    topLeft,topMiddle,topRight,bottomLeft,bottomRight,batterylvl, miclvl = read_serial.split(",")
    return int(topLeft),int(topMiddle),int(topRight),int(bottomLeft),int(bottomRight),float(batterylvl), int(miclvl)

#returns a tuple with the data as integers
def requestData():
    #ON linky only speed is currently requested

    ser.write("Z")#REQUEST INFO
    read_serial = ser.readline()
    #read the Serial line the rokit should be sending a message

    #chop up that message
    servo1Angle,servo2Angle,servoStep,speed,a19,a20,a21 = read_serial.split(" ")
    #now return the data as integers
    return int(servo1Angle),int(servo2Angle),int(servoStep),int(speed),int(a19),int(a20),int(a21)    


def forward():
    ser.write('M')

def backward():
    ser.write('N')

def left():
    ser.write('O')

def right():
    ser.write('P')
    
def stop():
    ser.write('Q')


def speedDecrease():
    ser.write('V')

def speedIncrease():
    ser.write('W')


def speed(speedPercentage):
    
    servo1Angle,servo2Angle,servoStep,speed,a19,a20,a21 = requestData()
    currentSpeed = speed
    
    while(currentSpeed > speedPercentage):
        servo1Angle,servo2Angle,servoStep,speed,a19,a20,a21 = requestData()
        currentSpeed = speed
        speedDecrease()
    while(currentSpeed < speedPercentage):
        servo1Angle,servo2Angle,servoStep,speed,a19,a20,a21 = requestData()
        currentSpeed = speed
        speedIncrease()
    
    




