# this program reads the serial port
# at a baud rate of 115200

import serial

ser = serial.Serial('/dev/ttyUSB0', 115200)


# returns only 3 of 7 bottom IR sensors
def readBottomIRSensors():
    ser.write("L")
    read_serial = ser.readline()
    d11, d12, d13, d14, d16, d17, d18 = read_serial.split(" ")
    return int(d11), int(d14), int(d18)


# this request all the bottom IR sensors
def requestBottomIRSensors():
    ser.write("L")
    read_serial = ser.readline()
    d11, d12, d13, d14, d16, d17, d18 = read_serial.split(" ")
    return int(d11), int(d12), int(d13), int(d14), int(d16), int(d17), int(d18)


# returns a tuple with the data as integers
def requestData():
    ser.write("Z")  # REQUEST INFO
    read_serial = ser.readline()
    # read the Serial line the rokit should be sending a message

    # chop up that message
    servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = read_serial.split(" ")
    # now return the data as integers
    return int(servo1Angle), int(servo2Angle), int(servoStep), int(speed), int(a19), int(a20), int(a21)


def requestMotorSpeed():
    ser.write("Z")  # REQUEST INFO
    read_serial = ser.readline()
    # read the Serial line the rokit should be sending a message

    # chop up that message
    servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = read_serial.split(" ")
    # now return the data as integers
    return int(speed)


# returns a tuple with the data as integers
def readTopIRsensors():
    servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = requestData()
    # now return the strings as integers
    return int(a19), int(a20), int(a21)


def moveForwards():
    ser.write('M')


def moveBackwards():
    ser.write('N')


def moveLeft():
    ser.write('O')


def moveRight():
    ser.write('P')


def moveStop():
    ser.write('Q')


def servo1Decrease():
    ser.write('R')


def servo1Increase():
    ser.write('S')


def servo2Decrease():
    ser.write('T')


def servo2Increase():
    ser.write('U')


def speedDecrease():
    ser.write('V')


def speedIncrease():
    ser.write('W')


def servoStepIncrease():
    ser.write('X')


def servoStepDecrease():
    ser.write('Y')


def speed(speedPercentage):
    servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = requestData()
    currentSpeed = speed

    while (currentSpeed > speedPercentage):
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = requestData()
        currentSpeed = speed
        # print(currentSpeed)
        speedDecrease()
    while (currentSpeed < speedPercentage):
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = requestData()
        currentSpeed = speed
        # print(currentSpeed)
        speedIncrease()


# Sets the motor speed to a value of 0-100
def setSpeed(i):
    if i < 0:
        i = 0
    if i > 100:
        i = 100
    j = i % 10
    setModifier((i - j) / 10)
    ser.write('l')
    setModifier(j)
    ser.write('m')


# Plays a sound at frequency i and tempo j
# i is any value from 100-10000 in multiples of 100
# j is any value from 0-10
def buzz(i, j):
    if i < 100:
        i = 100
    if i > 10000:
        i = 10000
    if j < 0:
        j = 0
    if j > 10:
        j = 10
    k = i % 1000
    setModifier((i - k) / 1000)
    ser.write('n')
    i = k
    k = i % 100
    setModifier((i - k) / 100)
    ser.write('o')
    setModifier(j)
    ser.write('p')


def setMotor(i, j):
    """Sets the individual speed of each motor.
    Args: 2 integers, from -100 to 100, for the left and right motors, respectively.
    """
    reverse1 = 0
    reverse2 = 0
    if i < -100:
        i = -100
    if i < 0:
        i *= -1
        reverse1 = 1
    if i > 100:
        i = 100
    if j < -100:
        j = -100
    if j < 0:
        j *= -1
        reverse2 = 2
    if j > 100:
        j = 100
    k = i % 10
    setModifier((i - k) / 10)
    ser.write('q')
    setModifier(k)
    ser.write('r')
    k = j % 10
    setModifier((j - k) / 10)
    ser.write('s')
    setModifier(k)
    ser.write('t')
    setModifier(reverse1 + reverse2)
    ser.write('u')


def setModifier(i):
    ser.write(chr(i + 97))
