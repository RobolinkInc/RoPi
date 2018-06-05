import RoPi as rp
import cv2
import numpy as np
import time
import random

def left():
    ropi.setMotor(-20,20)

def right():
    ropi.setMotor(20,-20)

def forward():
    ropi.setMotor(34,30)

#Constants
RESOLUTION_W = 320
RESOLUTION_H = 240
ERROR = 40

#Variables
turnLeft = 0
atIntersection = 0
atCorner = 0
doOnce = 0
cTime = 0
direction = 0
directionTemp = 0
instructionsIter = 0
isParking = 0
spotIter = 0

#Initialize
instructions = np.array([0,3])
ropi = rp.RoPi(speed=20,width=RESOLUTION_W,height=RESOLUTION_H)
random.seed()

SEGMENTS_LOOKUP = {
        (1, 1, 1): 1, # -| or |-
        (1, 2, 0): 2, #_|_
        (2, 1, 0): 3, #T
        (1, 2, 1): 4, #+
}

while(1):
    #Quit if user presses "q" inside display
    key = cv2.waitKey(1) & 0xFF
    
    if key == ord('q'):
        break

    #lIR, mIR, rIR = ropi.readTopIRsensors()

    #if lIR < 200:
    #    right()
    #    time.sleep(0.2)

    #if rIR < 200:
    #    left()
    #    time.sleep(0.2)
    
    #Get frame and flip
    frame = ropi.getFrame()
    frame = cv2.flip(frame, -1)

    cv2.imshow("Live Feed", frame)

    #When you've reached the target node, start searching for yellow contours
    if instructionsIter > len(instructions) - 1:
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        lower = np.array([20,70,130])
        upper = np.array([35,195,255])
        yellow = cv2.inRange(hsv, lower, upper)

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        blur = cv2.GaussianBlur(gray, (5, 5), 0)
        mask = cv2.bitwise_and(yellow, blur)

        cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE) [-2]

        #Initialize array
        arrows = []

        #Only look at contours larger than 300 pixels
        for c in cnts:
            if cv2.contourArea(c) > 300:
                arrows.append(c)
                
        if len(arrows) > 0:
            #Enter "Parking Mode" -- Ignore blue contours
            isParking = 1
            c = min(arrows, key=cv2.contourArea)
            x, y, w, h = cv2.boundingRect(c)
            #DEBUG. Display centroids of contours and print width
            #cv2.circle(frame, (int(x+w/2), int(y+h/2)), 7, (255, 255, 255), -1)
            #cv2.imshow("Result", frame)
            #print(w)
            #If you see a contour that takes up most of the screen, that is most likely the parking space
            if w > RESOLUTION_W*0.7:
                forward()
                time.sleep(1.5)
                ropi.moveStop()
                time.sleep(1)
                ropi.setMotor(-30,30)
                time.sleep(3)
                if spotIter == 0:
                    turnLeft == 1
                    direction = 2
                    instructions = np.array([1,1])
                elif spotIter == 1:
                    turnLeft == 0
                    direction = 3
                    instructions = np.array([3])
                elif spotIter == 2:
                    turnLeft = 0
                    direction = 1
                    instructions = np.array([1,0])
                else:
                    break
                spotIter += 1
                instructionsIter = 0
                isParking = 0
            #Move towards yellow contour
            if x + w/2 < RESOLUTION_W/3:
                turnLeft = 1
                left()
            elif x + w/2 < RESOLUTION_W*2/3:
                forward()
            else:
                turnLeft = 0
                right()
            continue
        else:
            #If you don't see any yellow contours, keep spinning around until you do
            if isParking == 1:
                if turnLeft == 1:
                    ropi.setMotor(-30,30)
                    #left()
                else:
                    ropi.setMotor(30,-30)
                    #right()

    #When "Parking Mode" is enabled, blue contours are ignored
    if isParking > 0:
        continue

    #Filter out all colors except blue
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    #lower = np.array([85,65,75]) #Colors used for the office
    #upper = np.array([140,185,150]) #Colors used for the office
    lower = np.array([100,100,75])
    upper = np.array([125,230,150])
    blue = cv2.inRange(hsv, lower, upper)

    #Gray and blur
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    mask = cv2.bitwise_and(blue, blur)

    #Find contours
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE) [-2]

    #Initialize array
    lanes = []
    
    #Filter out all contours except the ones that are big and towards the bottom of the screen
    for c in cnts:
        x, y, w, h = cv2.boundingRect(c)
        if cv2.contourArea(c) > 400 and (y+h/2) > RESOLUTION_H/3:
            lanes.append(c)

    #If no lanes found, skip the rest of the code
    #If you're not at an intersection, keep spinning until you see a contour. After 4 seconds, reverse direction
    if len(lanes) == 0:
        if atIntersection == 0:
            if doOnce == 0:
                cTime = time.time()
                doOnce = 1
            if turnLeft == 0:
                if time.time() - cTime < 4:
                    right()
                else:
                    ropi.setMotor(-30,30)
            else:
                if time.time() - cTime < 4:
                    left()
                else:
                    ropi.setMotor(30,-30)
            #cv2.imshow("Result", frame)
        continue

    if atIntersection == 2:
        atIntersection = 0

    #Reset timer that tells the drone to reverse directions
    doOnce = 0

    #Initialize arrays
    regionTemp = []
    region1 = []
    region2 = []
    region3 = []
    
    #Sort contours, top-to-bottom
    boundingBoxes = [cv2.boundingRect(c) for c in lanes]
    (lanes, boundingBoxes) = zip(*sorted(zip(lanes, boundingBoxes),
        key=lambda b:b[1][1], reverse=False))

    x1, y1, w1, h1 = cv2.boundingRect(lanes[0])
    region1.append(lanes[0])

    #DEBUG. Display centroids for all contours
    #for c in lanes:
    #    x, y, w, h = cv2.boundingRect(c)
    #    cv2.circle(frame, (int(x+w/2), int(y+h/2)), 7, (255, 255, 255), -1)
    
    if len(region1) > 0:
        for i in range(1,len(lanes)):
            x, y, w, h = cv2.boundingRect(lanes[i])
            if (y+h/2) > (y1+h1/2) - 10 and (y+h/2) < (y1+h1/2) + 10:
                region1.append(lanes[i])
            else:
                regionTemp.append(lanes[i])

    if len(regionTemp) > 0:
        x, y2, w, h2 = cv2.boundingRect(regionTemp[0])
        region2.append(regionTemp[0])
        if len(region2) > 0:
            for i in range(1,len(regionTemp)):
                x, y, w, h = cv2.boundingRect(regionTemp[i])
                if (y+h/2) > (y2+h2/2) - 10 and (y+h/2) < (y2+h2/2) + 10:
                    region2.append(regionTemp[i])
                else:
                    region3.append(regionTemp[i])

    if len(region2) > 0:
        for c in region2:
            x2, y, w2, h = cv2.boundingRect(c)

    if len(region3) > 0:
        for c in region3:
            x3, y, w3, h = cv2.boundingRect(c)

    segments = [len(region1), len(region2), len(region3)]
    intersection = SEGMENTS_LOOKUP.get(tuple(segments), 0)

    #Get center of intersection
    if intersection == 1:
        x, y, w, h = cv2.boundingRect(region2[0])
        x10, y, w10, h = cv2.boundingRect(region1[0])
        x11, y, w11, h = cv2.boundingRect(region3[0])
        if (x2+w2/2) < (x1+w1/2) - ERROR and (x2+w2/2) < (x3+w3/2) - ERROR:
            intersection = 10
        elif (x2+w2/2) > (x1+w1/2) + ERROR and (x2+w2/2) > (x3+w3/2) + ERROR:
            intersection = 11
        else:
            intersection = 0
    elif intersection == 2:
        x10, y, w10, h = cv2.boundingRect(region2[0])
        x11, y, w11, h = cv2.boundingRect(region2[1])
    elif intersection == 3:
        x10, y, w10, h = cv2.boundingRect(region1[0])
        x11, y, w11, h = cv2.boundingRect(region1[1])
        if (y1+h1/2) < RESOLUTION_H*3/5:
            forward()
    elif intersection == 4:
        x10, y, w10, h = cv2.boundingRect(region2[0])
        x11, y, w11, h = cv2.boundingRect(region2[1])

    if intersection == 0:
        if atIntersection == 1:
            #Follow the instructions to determine how you need to turn
            if instructionsIter <= len(instructions) - 1:
                x = instructions[instructionsIter]
                directionTemp = x
                if x == 3:
                    x -= 4
                if direction == 3:
                    direction -= 4
                if x < direction:
                    forward()
                    time.sleep(0.7)
                    left()
                    time.sleep(3)
                elif x > direction:
                    forward()
                    time.sleep(0.7)
                    right()
                    time.sleep(3)
                else:
                    forward()
                    time.sleep(0.7)
                direction = directionTemp
                instructionsIter += 1
            else:
                #Let Jesus take the wheel
                while 1:
                    a = random.randint(0,2)
                    if intersection == 10 and a == 2:
                        continue
                    if intersection == 11 and a == 0:
                        continue
                    if intersection == 3 and a == 1:
                        continue
                    break
                if a == 0:
                    forward()
                    time.sleep(0.7)
                    left()
                    time.sleep(3)
                elif a == 1:
                    forward()
                    time.sleep(0.7)
                else:
                    forward()
                    time.sleep(0.7)
                    right()
                    time.sleep(3)
                    
            atIntersection = 2
            continue

        #On straightaways, get centroid and angle for closest centroid
        x, y, w, h = cv2.boundingRect(lanes[-1])
        rect = cv2.minAreaRect(lanes[-1])
        a = rect[2]

        atCorner = 0

        if a < -60:
            turnLeft = 1
            atCorner = 1

        if a > -30 and a < -0:
            turnLeft = 0
            atCorner = 1

        #Follow centroid. If at a corner, go around it.
        if x < RESOLUTION_W/3:
            if atCorner == 0:
                left()
            else:
                if turnLeft == 1:
                    forward()
                    time.sleep(0.5)
        elif x < RESOLUTION_W*2/3:
            if atCorner == 0:
                forward()
            else:
                if turnLeft == 1:
                    right()
                else:
                    left()
        else:
            if atCorner == 0:
                right()
            else:
                if turnLeft == 0:
                    forward()
                    time.sleep(0.5)
    else:
        #At intersections, follow the center of the intersection
        x = ((x10+w10/2) + (x11+w11/2)) / 2

        #Debug. Display centroid
        #cv2.circle(frame, (int(x+w/2), int(y+h/2)), 7, (255, 255, 255), -1)

        if x < RESOLUTION_W/3:
            left()
        elif x < RESOLUTION_W*2/3:
            forward()
        else:
            right()
        atIntersection = 1

    #cv2.imshow("Result", frame)

ropi.moveStop()
cv2.destroyAllWindows()
