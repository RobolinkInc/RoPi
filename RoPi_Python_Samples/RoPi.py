
import serial
from picamera.array import PiRGBArray
from picamera import PiCamera
import time # Time library for delays
import sys
sys.path.append('/usr/local/lib/python3.4/site-packages')


class RoPi:
    def __init__(self,speed=30,width=320,height=240):
        # Reads the serial port at a baud rate of 115200
        self.ser = serial.Serial('/dev/ttyUSB0', 115200)

        self.width = width
        self.height = height

        # Initialize the camera
        self.camera = PiCamera()
        self.resolution = (width, height)
        self.camera.resolution = self.resolution
        self.camera.framerate = 32   # set frame rate to 32 frames per second

        # Grab a reference to the raw camera capture
        self.rawCapture = PiRGBArray(self.camera, size=self.resolution)
        time.sleep(0.1)

        # Capture frames from the camera
        self.stream = self.camera.capture_continuous(self.rawCapture, format="bgr", use_video_port=True)
        next(self.stream)
        # Set the speed
        self.speed(speed)

    def getFrame(self):
        # Resize the stream to the given size
        self.rawCapture.truncate(0)
        frame = next(self.stream)
        return frame.array

    def readBottomIRSensors(self,):
        """Returns only 3 of 7 bottom IR sensors
        Returns: A tuple with the data as integers. Left, middle, right bottom IR sensors.
        """
        self.ser.write(b"L")
        read_serial = self.ser.readline()
        d11, d12, d13, d14, d16, d17, d18 = read_serial.split()
        return int(d11), int(d14), int(d18)


    def requestBottomIRSensors(self,):
        """This function request all the bottom IR sensors
        Returns: A tuple with the data as integers. All the bottom IR sensors from left to right.
        """
        self.ser.write(b"L")
        read_serial = self.ser.readline()
        d11, d12, d13, d14, d16, d17, d18 = read_serial.split()
        return int(d11), int(d12), int(d13), int(d14), int(d16), int(d17), int(d18)


    def requestData(self,):
        """This function request motor and top IR sensors data.
        Returns: A tuple with the data as integers.
                servo left angle, servo right angle, servo step, motor speed, top ir left sensor, top ir middle sensor, top ir right sensor
        """
        self.ser.write(b"Z")  # REQUEST INFO
        read_serial = self.ser.readline()
        # read the Serial line the rokit should be sending a message
        # chop up that message
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = read_serial.split()
        # now return the data as integers
        return int(servo1Angle), int(servo2Angle), int(servoStep), int(speed), int(a19), int(a20), int(a21)


    def requestMotorSpeed(self,):
        """This function request motor speed data.
        Return: A motor speed data as integers.
        """
        self.ser.write(b"Z")  # REQUEST INFO
        read_serial = self.ser.readline()
        # read the Serial line the rokit should be sending a message

        # chop up that message
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = read_serial.split()
        # now return the data as integers
        return int(speed)


    def readTopIRsensors(self,):
        """
        Returns: A tuple with the data as integers. Left, middle, right top IR sensors.
        """
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = self.requestData()
        # now return the strings as integers
        return int(a19), int(a20), int(a21)


    def moveForward(self,):
        self.ser.write(b'M')


    def moveBackward(self,):
        self.ser.write(b'N')


    def moveLeft(self,):
        self.ser.write(b'O')


    def moveRight(self,):
        self.ser.write(b'P')


    def moveStop(self,):
        self.ser.write(b'Q')


    def servo1Decrease(self,):
        self.ser.write(b'R')


    def servo1Increase(self,):
        self.ser.write(b'S')


    def servo2Decrease(self,):
        self.ser.write(b'T')


    def servo2Increase(self,):
        self.ser.write(b'U')


    def speedDecrease(self,):
        self.ser.write(b'V')


    def speedIncrease(self,):
        self.ser.write(b'W')


    def servoStepIncrease(self,):
        self.ser.write(b'X')


    def servoStepDecrease(self,):
        self.ser.write(b'Y')


    def speed(self,speedPercentage):
        """Sets the current speed
        Args: A value of 0-100 as integer.
        """
        servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = self.requestData()
        currentSpeed = speed

        while (currentSpeed > speedPercentage):
            servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = self.requestData()
            currentSpeed = speed
            # print(currentSpeed)
            self.speedDecrease()
        while (currentSpeed < speedPercentage):
            servo1Angle, servo2Angle, servoStep, speed, a19, a20, a21 = self.requestData()
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
        self.setModifier((speed - j) // 10)
        self.ser.write(b'l')
        self.setModifier(j)
        self.ser.write(b'm')


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
        self.setModifier((freq - k) // 1000)
        self.ser.write(b'n')
        freq = k
        k = freq % 100
        self.setModifier((freq - k) // 100)
        self.ser.write(b'o')
        self.setModifier(tempo)
        self.ser.write(b'p')


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
        self.setModifier((i - k) // 10)
        self.ser.write(b'q')
        self.setModifier(k)
        self.ser.write(b'r')
        k = j % 10
        self.setModifier((j - k) // 10)
        self.ser.write(b's')
        self.setModifier(k)
        self.ser.write(b't')
        self.setModifier(reverse1 + reverse2)
        self.ser.write(b'u')


    def setModifier(self,i):
        if i == 0:
            self.ser.write(b'a')
        elif i == 1:
            self.ser.write(b'b')
        elif i == 2:
            self.ser.write(b'c')
        elif i == 3:
            self.ser.write(b'd')
        elif i == 4:
            self.ser.write(b'e')
        elif i == 5:
            self.ser.write(b'f')
        elif i == 6:
            self.ser.write(b'g')
        elif i == 7:
            self.ser.write(b'h')
        elif i == 8:
            self.ser.write(b'i')
        elif i == 9:
            self.ser.write(b'j')
        else:
            self.ser.write(b'k')
