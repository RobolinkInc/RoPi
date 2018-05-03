'''
This program will create trackbars that will allow you to
find the HSV lower and upper bounds that contain the color you are
looking for.
In order to run, open with IDLE and then go into run
then click run module
to exit click exit on the python IDLE shell
'''

import RoPi as rp
import cv2

ropi = rp.RoPi()

def nothing(x):
    pass


# Creating a window for later use
cv2.namedWindow('Control Panel')

# Creating track bar
#cv.CreateTrackbar(trackbarName, windowName, value, count, onChange)  None
cv2.createTrackbar('hue', 'Control Panel',77,180,nothing)
cv2.createTrackbar('sat', 'Control Panel',133,255,nothing)
cv2.createTrackbar('val', 'Control Panel',140,255,nothing)
cv2.createTrackbar('hueRange', 'Control Panel',14,127,nothing)
cv2.createTrackbar('satRange', 'Control Panel',69,127,nothing)
cv2.createTrackbar('valRange', 'Control Panel',69,127,nothing)

 
while True:
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
    
    # grab the raw NumPy array representing the image - this array
    # will be 3D, representing the width, height, and # of channels
    #convert the image into a numpy array
    frame = ropi.getFrame()

    #flips the frame since the camera is upside down
    frame = cv2.flip(frame,0)
    
    #turn into HSV color space
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    #mask will be a frame that has filtered the color we are looking for
    #that color will be within the hsvLower and hsvUpper constraints.
    mask = cv2.inRange(hsv, hsvLower, hsvUpper)
    
    #this "res" frame pieces together 2 the mask frame and the original frame
    res = cv2.bitwise_and(frame,frame, mask= mask)

    #save the "key pressed" as a variable
    key = cv2.waitKey(1) & 0xFF
 
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break
    if key == ord("Q"):
        break


    cv2.imshow("frame", frame)
    cv2.imshow("res", res)

    e2 = cv2.getTickCount()
    time = (e2 - e1)/cv2.getTickFrequency()
    print("milliSeconds" , time*1000)

cv2.destroyAllWindows()
