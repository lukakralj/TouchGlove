from imutils.video import VideoStream
import numpy as np
import cv2
import imutils
import time
import subprocess
import os

lower = np.array([110, 100, 100])
upper = np.array([130, 255, 255])
frameW = 600
frameH = -1 # calculated below
moveTreshold = 3 #pixels
screenBorder = 40 #pixels

# get screen size
cmd = ['xrandr']
cmd2 = ['grep', '*']
p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
p2 = subprocess.Popen(cmd2, stdin=p.stdout, stdout=subprocess.PIPE)
p.stdout.close()
resolution_string, junk = p2.communicate()
resolution = resolution_string.split()[0]
screenW, screenH = resolution.split('x')
screenW = int(screenW) - screenBorder
screenH = int(screenH) - screenBorder

frameH = int(float(frameW * screenH) / float(screenW))

XscaleFactor = float(screenW) / float(frameW)
YscaleFactor = float(screenH) / float(frameH)

started = False
prevCenter = None
prevRadius = -1
def processMove(center, radius):
    if not started: 
        return
    global prevCenter
    global prevRadius
    #print("prev: {}, cur: {}".format(prevCenter, center))
    if center is None:
        prevCenter = None
        prevRadius = -1
    elif prevCenter is None:
        prevCenter = center
        prevRadius = radius
    else:
        diffX = float(center[0]) - float(prevCenter[0])
        diffY = float(center[1]) - float(prevCenter[1])
        #print("diff: {}, {} ".format(diffX, diffY))
        if abs(diffX) < moveTreshold:
            diffX = 0
        if abs(diffY) < moveTreshold:
            diffY = 0
        
        diffX *= XscaleFactor * -1 # multiply by -1 to change direction because of mirroring
        diffY *= YscaleFactor

        if diffX != 0 or diffY != 0:
            cmd = 'xte "mousermove {} {}"'.format(int(diffX), int(diffY))
            os.system(cmd)
            prevCenter = center
            prevRadius = radius


vs = VideoStream(src=0).start()
time.sleep(2.0)

while True:
    frame = vs.read()

    if frame is None:
        break

    # resize the frame, blur it, and convert it to the HSV
    # color space
    frame = imutils.resize(frame, width=frameW, height=frameH)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # construct a mask for the color "green", then perform
    # a series of dilations and erosions to remove any small
    # blobs left in the mask
    mask = cv2.inRange(hsv, lower, upper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
                            cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None

    # only proceed if at least one contour was found
    if len(cnts) > 0:
        # find the largest contour in the mask, then use it to compute the minimum enclosing circle and centroid
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

        # only proceed if the radius meets a minimum size
        if radius > 10:
            started = True
            # draw the circle and centroid on the frame
            cv2.circle(frame, (int(x), int(y)), int(radius),
                       (0, 255, 255), 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)

            processMove((x,y), radius)

    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF

    if key == ord("q"):
        break

vs.stop()
cv2.destroyAllWindows()
