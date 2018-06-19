import os
import sys

import RoPi_SerialCom as ropi

sys.path.append('/usr/local/lib/python2.7/site-packages')

from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import numpy as np

RESOLUTION_W = 160
RESOLUTION_H = 128

camera = PiCamera()
camera.resolution = (RESOLUTION_W, RESOLUTION_H)
camera.hflip = True
camera.vflip = True
rawCapture = PiRGBArray(camera)
time.sleep(2)

for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
        frame = np.array(frame.array)

        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        #using yellow
        #lower = np.array([0,0,140])
        #lower = np.array([2,255,171])
        lower = np.array([2,197,128])
        #upper = np.array([80,100,255])
        #upper = np.array([110,255,221])
        upper = np.array([30, 255, 255])
        mask = cv2.inRange(hsv, lower, upper)

        cv2.line(mask, (RESOLUTION_W/3,0), (RESOLUTION_W/3,RESOLUTION_H),(0,0,0),4,8,0)
        cv2.line(mask, (RESOLUTION_W*2/3,0), (RESOLUTION_W*2/3,RESOLUTION_H),(0,0,0),4,8,0)

        cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE) [-2]
        cntSort = []

        if len(cnts) > 0:
            for c in cnts:
                if cv2.contourArea(c) > 200:
                    cntSort.append(c)

        if len(cntSort) > 1:
            cntMax = max(cnts, key=cv2.contourArea)
            xM,y,wM,h = cv2.boundingRect(cntMax)
            if xM+wM/2 < RESOLUTION_W/3:
                regionMax = 0
            elif xM+wM/2 < RESOLUTION_W*2/3:
                regionMax = 1
            else:
                regionMax = 2

            yL = 0
            hL = 0
            yR = 0
            hR = 0
            left = 0
            right = 0

            for c in cntSort:
                x,y,w,h = cv2.boundingRect(c)
                if w < RESOLUTION_W/6:
                    continue
                if x+w/2 < RESOLUTION_W/3:
                    region = 0
                elif x+w/2 < RESOLUTION_W*2/3:
                    region = 1
                else:
                    region = 2
                if region < regionMax:
                    yL = y
                    hL = h
                    left = 1
                if region > regionMax:
                    yR = y
                    hR = h
                    right = 1

            if left == 1 and right == 1:
                x = xM
                w = wM
                y = (yL+yR)/2
                h = (hL+hR)/2
                cv2.circle(frame, (x+w/2, y+h/2), 7, (255, 255, 255), -1)
                print("T-section")

            if left == 0 and right == 1:
                x = xM
                w = wM
                y = yR
                h = hR
                cv2.circle(frame, (x+w/2, y+h/2), 7, (255, 255, 255), -1)
                print("Right")
                ropi.confirm()
                ropi.stop()
                ropi.stumble()
                time.sleep(1)
                ropi.stop()
                ropi.right()
                time.sleep(0.28)
                ropi.stop()
                ropi.stumble()
                time.sleep(1)
                ropi.stop()
                print("done turning")
            if left == 1 and right == 0:
                x = xM
                w = wM
                y = yL
                h = hL
                cv2.circle(frame, (x+w/2, y+h/2), 7, (255, 255, 255), -1)
                print("Left")
                ropi.confirm()
                ropi.stop()
                ropi.stumble()
                time.sleep(1)
                ropi.stop()
                ropi.left()
                time.sleep(0.28)
                ropi.stop()
                ropi.stumble()
                time.sleep(1)
                ropi.stop()
                print("done turning")

        ropi.forward()

        cv2.imshow("Result", frame)

        rawCapture.truncate(0)

        key = cv2.waitKey(1) & 0xFF

        if key == ord("q"):
                        break

cv2.destroyAllWindows()
