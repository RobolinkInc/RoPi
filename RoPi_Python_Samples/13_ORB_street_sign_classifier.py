import cv2
import numpy as np
#https://docs.opencv.org/3.0-beta/doc/py_tutorials/py_feature2d/py_matcher/py_matcher.html

# Initialize camera, ORB detector, and BFMatcher object
cap = cv2.VideoCapture(0)
orb = cv2.ORB_create(nfeatures=100000, scoreType=cv2.ORB_FAST_SCORE)
#bf = cv2.BFMatcher()
bfH = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)
MIN_MATCHES = 90
#bfH2 = cv2.BFMatcher(cv2.NORM_HAMMING2, crossCheck=True)
#bf = cv2.BFMatcher(cv2.NORM_L1, crossCheck=True)


# Define the object in the image we are looking for
img1 = cv2.imread('stop_sign_train.jpg', 0)

#resize image
divideFrameBy = 4
height, width = img1.shape;
img1 = cv2.resize(img1, (int(width / divideFrameBy), int(height / divideFrameBy)))

# Get keypoints and descriptors for sample image (what we are looking for)
kp1, des1 = orb.detectAndCompute(img1, None)


def nothing(x):
    pass

# Creating a window for later use
cv2.namedWindow('Control Panel')

cv2.createTrackbar('numberOfMatches', 'Control Panel', 2, 1000, nothing)
cv2.createTrackbar('ratio Test', 'Control Panel', 7, 1000, nothing)
cv2.createTrackbar('value 3', 'Control Panel', 1, 1000, nothing)
cv2.createTrackbar('value 4', 'Control Panel', 2, 1000, nothing)



while (1):

    numberOfMatches = cv2.getTrackbarPos('numberOfMatches', 'Control Panel')
    ratioTest = cv2.getTrackbarPos('ratio Test', 'Control Panel')
    val_3 = cv2.getTrackbarPos('value 3', 'Control Panel')
    val_4 = cv2.getTrackbarPos('value 4', 'Control Panel')

    # Take each frame
    ret, frame = cap.read()
    height, width, layers = frame.shape

    frame = cv2.resize(frame, (int(width / divideFrameBy), int(height / divideFrameBy)))

    #convert image to gray scale
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Get keypoints and descriptors for video frame
    kp2, des2 = orb.detectAndCompute(gray, None)

    # Match descriptors
    matches = bfH.match(des1, des2)

    '''
    matches.distance - Distance between descriptors. The lower, the better it is.
    matches.trainIdx - Index of the descriptor in train descriptors
    matches.queryIdx - Index of the descriptor in query descriptors
    matches.imgIdx - Index of the train image.
    '''
    #sort that list of "matches" by distance from smallest to biggest
    matches = sorted( matches , key=lambda x: x.distance)

    #bf.match return only a list of single objects

    # Draw first matches which wll be the first smallest distances
    orbMatches = cv2.drawMatches(img1, kp1, gray, kp2, matches[:numberOfMatches], None, flags=2)


    #verify if there are enough matches for a good amtch
    goodMatch = []
    for m in matches:
        if m.distance < 60: #tested to ensure a bag of Cheez-Its wouldn't be a false positive
            goodMatch.append(m)
    if (len(goodMatch) > MIN_MATCHES):
        print("WOWOW a good match!!")
    else:
        #print("not a good match..")
        pass

    # Display result
    cv2.imshow("Orb Matches", orbMatches)

    # Break on key press
    k = cv2.waitKey(1) & 0xFF
    if k == 27:
        break  # Destroy all windows
cv2.destroyAllWindows()
