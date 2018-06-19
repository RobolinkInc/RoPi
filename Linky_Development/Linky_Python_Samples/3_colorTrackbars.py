'''
This program will create trackbars that will allow you to
find the hue, saturation, and value lower and upper bounds
that contain the color you are looking for.
In order to run this program, open with IDLE and then go into run
then click run module
to exit click exit on the python IDLE shell
'''

import sys
sys.path.append('/usr/local/lib/python2.7/site-packages')

#this is for the pi camera to work
from picamera.array import PiRGBArray
from picamera import PiCamera

# time library for delays
import time

#OpenCV library
import cv2

#Math library
import numpy as np 

#an empty function used for createTrackbar()
def nothing(x):
    pass


# Creating a window for later use
cv2.namedWindow('Control Panel')

# Creating track bar
#cv.CreateTrackbar(trackbarName, windowName, initialValue, rangeOfValue, onChange)
cv2.createTrackbar('hue', 'Control Panel',77,180,nothing)
cv2.createTrackbar('sat', 'Control Panel',133,255,nothing)
cv2.createTrackbar('val', 'Control Panel',140,255,nothing)
cv2.createTrackbar('hueRange', 'Control Panel',14,127,nothing)
cv2.createTrackbar('satRange', 'Control Panel',69,127,nothing)
cv2.createTrackbar('valRange', 'Control Panel',69,127,nothing)

 
#this is all to set up the pi camera
camera = PiCamera()
#resolution = (640, 480)
#resolution = (320, 240)
resolution = (160,128)
#resolution = (80,64)
#resolution = (48,32)
#to speed up the process use the lowest resolution

camera.resolution = resolution
#set frame rate to 32 frames per second
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=resolution)


#Allow the camera to warmup, if you dont sometimes the camera wont boot up
time.sleep(0.1)

# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):

    #grab intial time to measure how long
    #it takes to process the image
    e1 = cv2.getTickCount()

    #///////////////////////////////////////////////////////////////
    #collect all the trackbar positions
    h = cv2.getTrackbarPos('hue','Control Panel')
    s = cv2.getTrackbarPos('sat','Control Panel')
    v = cv2.getTrackbarPos('val','Control Panel')
    hr = cv2.getTrackbarPos('hueRange', 'Control Panel')
    sr = cv2.getTrackbarPos('satRange', 'Control Panel')
    vr = cv2.getTrackbarPos('valRange', 'Control Panel')

    #use the trackbar positions to set
    #a boundary for the color filter
    hsvLower = (h-hr, s-sr, v-vr)
    hsvUpper = (h+hr, s+sr, v+vr) 
    #///////////////////////////////////////////////////////////////

    #convert the image into a numpy array
    frame = np.array(frame.array)

    #flip the frame if you need to ex. change 0 to 1,2 etc.
    frame = cv2.flip(frame,-1)
    
    #turn into HSV color space, Hue Saturation and Value
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    #mask will be a frame that has filtered the color we are looking for
    #that color will be within the hsvLower and hsvUpper constraints.
    mask = cv2.inRange(hsv, hsvLower, hsvUpper)
    
    #this "res" frame pieces together 2 the mask frame and the original frame
    res = cv2.bitwise_and(frame,frame, mask= mask)

    #save the "key pressed" as a variable
    key = cv2.waitKey(1) & 0xFF
 
    # clear the stream in preparation for the next frame
    rawCapture.truncate(0)
 
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break
    if key == ord("Q"):
        break


    cv2.imshow("frame", frame)
    cv2.imshow("mask", mask)
    cv2.imshow("res", res)

    e2 = cv2.getTickCount()
    
    timeElapsed = int(1000*(e2 - e1)/cv2.getTickFrequency())
    print (timeElapsed, "ms")
# clear the stream in preparation for the next frame
rawCapture.truncate(0)
cv2.destroyAllWindows()
