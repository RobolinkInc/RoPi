#this program will try to follow an orange cone by using simple color detection
#adjust the trackbars to get a better filtered orange

import os
import sys
sys.path.append('/usr/local/lib/python2.7/site-packages')

#this is for the pi camera to work
from picamera.array import PiRGBArray
from picamera import PiCamera

# time library for delays
import time

import RoPi_SerialCom as ropi

#OpenCV library
import cv2

#math libraries
import numpy as np 


def nothing(x):
    pass


# Creating a window for later use
cv2.namedWindow('Control Panel')

# Creating track bar
#cv.CreateTrackbar(trackbarName, windowName, value, count, onChange)  None
cv2.createTrackbar('hue', 'Control Panel',180,180,nothing)
cv2.createTrackbar('sat', 'Control Panel',251,255,nothing)
cv2.createTrackbar('val', 'Control Panel',100,255,nothing)
cv2.createTrackbar('hueRange', 'Control Panel',5,127,nothing)
cv2.createTrackbar('satRange', 'Control Panel',120,127,nothing)
cv2.createTrackbar('valRange', 'Control Panel',80,127,nothing)

 
# The tutorial to set up the PI camera comes from here
# http://www.pyimagesearch.com/2016/08/29/common-errors-using-the-raspberry-pi-camera-module/ 
# initialize the camera and grab a reference to the raw camera capture
#this is all to set up the pi camera
camera = PiCamera()

#width in pixels of camera resolution
#resolution_w = 320
resolution_w = 160

threshold = 0.125*resolution_w

#height in pixels of camera resolution
#resolution_h = 240
resolution_h = 128

resolution = (resolution_w,resolution_h)

frame_width = resolution_w


camera.resolution = resolution
camera.hflip = True
camera.vflip = True
#set frame rate to 32 frames per second
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=resolution)


#Allow the camera to warmup, if you dont sometimes the camera wont boot up
time.sleep(0.1)

def filterColor(frame):
    
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
    
    #turn into HSV color space
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    #mask will be a frame that has filtered the color we are looking for
    #that color will be within the hsvLower and hsvUpper constraints.
    mask = cv2.inRange(hsv, hsvLower, hsvUpper)

    return mask


# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):

    #grab intial time to measure how long
    #it takes to process the image
    e1 = cv2.getTickCount()

    # grab the raw NumPy array representing the image - this array
    # will be 3D, representing the width, height, and # of channels
    #convert the image into a numpy array
    frame = np.array(frame.array)
    
    #///////////////////////////
    mask = filterColor(frame)
    #///////////////////////////
    
    #this "res" frame pieces together 2 the mask frame and the original frame
    res = cv2.bitwise_and(frame,frame, mask= mask)

    #find contours in the mask
    #cnts is countours
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]

    #-----------------------------------------------------------------------
    #Only proceed if at least one contour was found
    if len(cnts) > 0:
        # find the largest contour in the mask, then use
        # it to compute the minimum enclosing circle and
        # centroid
        c = max(cnts, key=cv2.contourArea)
        #find a minimum enclosing circle
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        cv2.circle(res, (int(x), int(y)), int(radius),(0, 255, 255), 2)
        
        #if the radius of that object is bigger than 15 pixels
        if(radius > 15):
            if x < (resolution_w/2-threshold):
                ropi.speed(20)
                ropi.left()
                time.sleep(0.05)
                ropi.stop()
                print("left", x)
            elif x > (resolution_w/2+threshold):
                ropi.speed(20)
                ropi.right()
                time.sleep(0.05)
                ropi.stop()
                print("right", x)
            else:
                print("centered", x) 
                ropi.speed(70)
                ropi.forward()
            

    else:
        #print("Nothing detected stopping")
        ropi.stop()
    #-----------------------------------------------------------------------                

    #the key that is clicked save it as variable key
    key = cv2.waitKey(1) & 0xFF
 
    # clear the stream in preparation for the next frame
    rawCapture.truncate(0)
 
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        ropi.stop()
        break



    cv2.imshow("frame", frame)
    cv2.imshow("res", res)

    e2 = cv2.getTickCount()
    timeElapsed = int(1000*(e2 - e1)/cv2.getTickFrequency())
    print (timeElapsed, "ms")
    
# clear the stream in preparation for the next frame
rawCapture.truncate(0)
cv2.destroyAllWindows()
#just in case the robot is still moving
ropi.stop()
print("End of Program")
            
