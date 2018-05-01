# this program reads the serial port
# at a baud rate of 115200

import serial

class RoPi:
    def __init__(self):
        self.ser = serial.Serial('/dev/ttyUSB0', 115200)

    def readBottomIRSensors(self,):
        """Returns only 3 of 7 bottom IR sensors
        Returns: A tuple with the data as integers. Left, middle, right bottom IR sensors.
        """
        self.ser.write("L")
        read_serial = self.ser.readline()
        d11, d12, d13, d14, d16, d17, d18 = read_serial.split(" ")
        return int(d11), int(d14), int(d18)


    def requestBottomIRSensors(self,):
        """This function request all the bottom IR sensors
        Returns: A tuple with the data as integers. All the bottom IR sensors from left to right.
        """
        self.ser.write("L")
        read_serial = self.ser.readline()
        d11, d12, d13, d14, d16, d17, d18 = read_serial.split(" ")
        return int(d11), int(d12), int(d13), int(d14), int(d16), int(d17), int(d18)


    def requestData(self,):
        """This function request motor and top IR sensors data.
        Returns: A tuple with the data as integers.
                servo left angle, servo right angle, servo step, motor speed, top ir left sensor, top ir middle sensor, top ir right sensor
        """
        self.ser.write("Z")  # REQUEST INFO
        read_serial = self.ser.readline()
        # read the Serial line the rokit should be sending a message

        # chop up that message
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = read_serial.split(" ")
        # now return the data as integers
        return int(servo1Angle), int(servo2Angle), int(servoStep), int(speed), int(a19), int(a20), int(a21)


    def requestMotorSpeed(self,):
        """This function request motor speed data.
        Return: A motor speed data as integers.
        """
        self.ser.write("Z")  # REQUEST INFO
        read_serial = self.ser.readline()
        # read the Serial line the rokit should be sending a message

        # chop up that message
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = read_serial.split(" ")
        # now return the data as integers
        return int(speed)


    def readTopIRsensors(self,):
        """
        Returns: A tuple with the data as integers. Left, middle, right top IR sensors.
        """
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = self.requestData()
        # now return the strings as integers
        return int(a19), int(a20), int(a21)


    def moveForwards(self,):
        self.ser.write('M')


    def moveBackwards(self,):
        self.ser.write('N')


    def moveLeft(self,):
        self.ser.write('O')


    def moveRight(self,):
        self.ser.write('P')


    def moveStop(self,):
        self.ser.write('Q')


    def servo1Decrease(self,):
        self.ser.write('R')


    def servo1Increase(self,):
        self.ser.write('S')


    def servo2Decrease(self,):
        self.ser.write('T')


    def servo2Increase(self,):
        self.ser.write('U')


    def speedDecrease(self,):
        self.ser.write('V')


    def speedIncrease(self,):
        self.ser.write('W')


    def servoStepIncrease(self,):
        self.ser.write('X')


    def servoStepDecrease(self,):
        self.ser.write('Y')


    def speed(self,speedPercentage):
        """Sets the current speed
        Args: A value of 0-100 as integer.
        """
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = requestData()
        currentSpeed = speed

        while (currentSpeed > speedPercentage):
            servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = requestData()
            currentSpeed = speed
            # print(currentSpeed)
            self.speedDecrease()
        while (currentSpeed < speedPercentage):
            servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = requestData()
            currentSpeed = speed
            # print(currentSpeed)
            self.speedIncrease()


    def setSpeed(self,speed):
        """Sets the motor speed.
        Args : A value of 0-100 as integer.
        """
        if speed < 0:
            speed = 0
        elif speed > 100:
            speed = 100
        j = speed % 10
        self.setModifier((speed - j) / 10)
        self.ser.write('l')
        self.setModifier(j)
        self.ser.write('m')


    def buzz(self, freq, tempo):
        """Plays a sound at frequency freq and tempo
        Args:   freq: Any value from 100-10000 in multiples of 100
                tempo: Any value from 0-10
        """
        if freq < 100:
            freq = 100
        if freq > 10000:
            freq = 10000
        if tempo < 0:
            tempo = 0
        if tempo > 10:
            tempo = 10
        k = freq % 1000
        self.setModifier((freq - k) / 1000)
        self.ser.write('n')
        freq = k
        k = freq % 100
        self.setModifier((freq - k) / 100)
        self.ser.write('o')
        self.setModifier(tempo)
        self.ser.write('p')


    def setMotor(self, i, j):
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
        self.setModifier((i - k) / 10)
        self.ser.write('q')
        self.setModifier(k)
        self.ser.write('r')
        k = j % 10
        self.setModifier((j - k) / 10)
        self.ser.write('s')
        self.setModifier(k)
        self.ser.write('t')
        self.setModifier(reverse1 + reverse2)
        self.ser.write('u')


    def setModifier(self,i):
        self.ser.write(chr(i + 97))
