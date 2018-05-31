#this program will try to follow an orange cone by using simple color detection
#adjust the trackbars to get a better filtered orange

import RoPi as rp
import cv2
import openCVHelper as cvHelper

frame_width = 320
frame_height = 240

#initiate the RoPi at speed 40% and set the width and height
ropi = rp.RoPi(speed= 40,width= frame_width,height= frame_height)


def nothing(x):
    pass

# Creating a window for later use
cv2.namedWindow('Control Panel')

# Creating track bar
#cv.CreateTrackbar(trackbarName, windowName, value, count, onChange)  None
cv2.createTrackbar('hue', 'Control Panel',3,180,nothing)
cv2.createTrackbar('sat', 'Control Panel',251,255,nothing)
cv2.createTrackbar('val', 'Control Panel',159,255,nothing)
cv2.createTrackbar('hueRange', 'Control Panel',5,127,nothing)
cv2.createTrackbar('satRange', 'Control Panel',69,127,nothing)
cv2.createTrackbar('valRange', 'Control Panel',69,127,nothing)

#--------------------------------------------------------------
def filterColor(frame):
    
    #///////////////////////////////////////////////////////////////
    #collect all the trackbar positions
    h = cv2.getTrackbarPos('hue','Control Panel')
    s = cv2.getTrackbarPos('sat','Control Panel')
    v = cv2.getTrackbarPos('val','Control Panel')
    hr = cv2.getTrackbarPos('satRange', 'Control Panel')
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

#--------------------------------------------------------------




while (1):
    #grab intial time to measure how long
    #it takes to process the image
    e1 = cv2.getTickCount()

    # grab the raw NumPy array representing the image - this array
    # will be 3D, representing the width, height, and # of channels
    #convert the image into a numpy array
    frame = ropi.getFrame()

    #flips the frame if necessary change 0 to 1 or 2..
    frame = cv2.flip(frame,0)
    
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
        maxCountour = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(maxCountour)
        cv2.circle(res, (int(x), int(y)), int(radius),(0, 255, 255), 2)
        
        #if the radius of that object is bigger than 5 pixels
        if(radius > 5):
            
            
            #if x pixel is below 38% of the frame width ex. x<(320*0.38=121.6)
            if(x<(frame_width*(0.5-0.12))):
                ropi.moveRight()
                print("Right")
                
                
            elif (x>(frame_width*(0.5+0.12))):
                ropi.moveLeft()
                print("Left")
                
            else:#object is centered stop
                ropi.moveStop()
                print("Stopping")
               
        else:#nothing is detected stop
            ropi.moveStop()
            print("Stop")
        
    #-----------------------------------------------------------------------                

    #the key that is clicked save it as variable key
    key = cv2.waitKey(1) & 0xFF
 
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break


    cv2.imshow("frame", frame)
    cv2.imshow("res", res)

    #store the time elapsed
    e2 = cv2.getTickCount()
    #(e2 - e1) gives you the number of ticks
    #this line converts the number of ticks into time elapsed per loop
    timeElapsed = int((e2 - e1)/cv2.getTickFrequency()*1000)
    print(timeElapsed,"ms")
    
#just in case the robot is still moving make it stop
ropi.moveStop()
#if click exit before clicking q, robot will still move
print("End of program")
cv2.destroyAllWindows()
