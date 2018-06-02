import cv2
import RoPi as rp
import numpy as np
from time import sleep

RESOLUTION_W = 320
RESOLUTION_H = 240

ropi = rp.RoPi(speed=20, width=RESOLUTION_W, height=RESOLUTION_H)

state = 0
ropi.speed(30)

while 1:
    frame = ropi.getFrame()
    frame = cv2.flip(frame, -1)

    #Filter out all colors except blue
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    lower = np.array([0,45,100])
    upper = np.array([30,240,185])
    yellow = cv2.inRange(hsv, lower, upper)

    #Gray and blur
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)

    #Combine yellow and blur into one mask
    mask = cv2.bitwise_and(yellow, blur)

    #Find contours
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE) [-2]

    cntSort = []

    #Get centroids for contours
    for c in cnts:
        if cv2.contourArea(c) > 200:
            cntSort.append(c)
    
    if len(cntSort) > 0:
        if state == 0:
            state = 1

        if state == 1:
            ropi.moveForward()
            
            parkDirection = 0
            
            for c in cntSort:
                x, y, w, h = cv2.boundingRect(c)
                if x+w/2 < RESOLUTION_W/2:
                    parkDirection -= 1
                else:
                    parkDirection += 1

            #if parkDirection < 0:
            #    print("Left")
            #else:
            #    print("Right")

        if state == 2:
            c = max(cntSort, key=cv2.contourArea)
            x, y, w, h = cv2.boundingRect(c)
            if w > RESOLUTION_W*0.9:
                state = 3

        if state == 3:
            ropi.moveForward()
    else:
        if state == 1:
            state = 2

        if state == 2:
            if parkDirection < 0:
                ropi.moveLeft()
                #print("Turning left...")
            else:
                ropi.moveRight()
                #print("Turning right...")

        if state == 3:
            print("How'd I do?")
            break

    #print(state)
    
    #Update windows
    #cv2.imshow("Mask", mask)
    cv2.imshow("Result", frame)

    #Press 'q' to quit
    key = cv2.waitKey(1) & 0xFF

    if key == ord("q"):
        break

#Stop drone and destroy windows
ropi.moveStop()
cv2.destroyAllWindows()
