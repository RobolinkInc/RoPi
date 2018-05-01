import sys
sys.path.append('/usr/local/lib/python2.7/site-packages')

#this is for the pi camera to work
from picamera.array import PiRGBArray
from picamera import PiCamera

# time library for delays
import time

#this is open CV library
import cv2
import numpy as np #math libraries
 
# The tutorial to set up the PI camera comes from here
# http://www.pyimagesearch.com/2016/08/29/common-errors-using-the-raspberry-pi-camera-module/ 
# initialize the camera and grab a reference to the raw camera capture
#this is all to set up the pi camera
camera = PiCamera()
resolution = (640, 480)
#resolution = (320, 240)
#resolution = (160,128)
#resolution = (80,64)
#resolution = (48,32)


camera.resolution = resolution
#set frame rate to 32 frames per second
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=resolution)


# allow the camera to warmup
time.sleep(0.1)

# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):

    # grab the raw NumPy array representing the image - this array
    # will be 3D, representing the width, height, and # of channels
    frame = np.array(frame.array)

    #flip the frame if you need to ex. change 0 to 1,2 etc.
    frame = cv2.flip(frame,0)

    # clear the stream in preparation for the next frame
    rawCapture.truncate(0)

    # show the frame
    cv2.imshow("Frame", frame)
    
    key = cv2.waitKey(1) & 0xFF
    
        # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break
    if key == ord("Q"):
        break

# clear the stream in preparation for the next frame
rawCapture.truncate(0)
cv2.destroyAllWindows()

 
