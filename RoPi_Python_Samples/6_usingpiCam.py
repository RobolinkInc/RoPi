import RoPi as rp
import cv2

ropi = rp.RoPi()

while True:
    # grab the raw NumPy array represeting the image - this array
    # will be 3D, representing the width, height, and # of channels
    frame = ropi.getFrame()

    #flip the frame if you need to ex. change 0 to 1,2 etc.
    frame = cv2.flip(frame,0)

    # show the frame
    cv2.imshow("Frame", frame)
    
    key = cv2.waitKey(1) & 0xFF
    
        # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break
    if key == ord("Q"):
        break

cv2.destroyAllWindows()

 
