from imutils.video import VideoStream
import numpy as np
import cv2
import imutils
import time
import subprocess
import os

#lower = np.array([110, 100, 100]) blue (often confused with black)
#upper = np.array([130, 255, 255]) blue
#lower = np.array([50, 100, 100]) #green (as in green-screen, did not match anything unless a very fake green (which is good))
#upper = np.array([70, 255, 255])
lower = np.array([75, 100, 100]) #more natural green
upper = np.array([95, 255, 255])
#lower = np.array([160, 100, 100]) #red (sometimes it interacts with some parts of skin)
#upper = np.array([180, 255, 255])


frameW = 600
frameH = -1 # calculated below
moveTreshold_lower = 5 #pixels
moveTreshold_upper = 30
radiusTreshold = 5 #pixels; move ignored if radius changes for more than this (to prevent flickering)
flickerFramesTreshold = 5 #number of frames in which the big increase in radius or move is ignored
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

XscaleFactor = (float(screenW) / float(frameW)) * -1 # multiply by -1 to change direction because of mirroring
YscaleFactor = float(screenH) / float(frameH)
scaleFactors = (XscaleFactor, YscaleFactor)

started = False
prevCenter = None
prevRadius = -1
curRadiusFlickers = 0
curMoveFlickers = [0, 0]
def processMove(center, radius):
    if not started: 
        return

    global prevCenter
    global prevRadius
    global curMoveXFlickers
    global curMoveYFlickers

    if center is None:
        prevCenter = None
        prevRadius = -1
    elif prevCenter is None:
        prevCenter = center
        prevRadius = radius
    else:
        diffX = calculateDiff(center, 0)
        diffY = calculateDiff(center, 1)  

        if isRadiusCorrect(radius) and (diffX != 0 or diffY != 0):
            cmd = 'xte "mousermove {} {}"'.format(int(diffX), int(diffY))
            os.system(cmd)
            prevCenter = center
            prevRadius = radius

# calculate the diff of the coordinate
# ind = 0 for X, ind = 1 for Y
# return float
def calculateDiff(center, ind):
    global curMoveFlickers
    diff = float(center[ind]) - float(prevCenter[ind])
    if abs(diff) < moveTreshold_lower:
        diff = 0
        curMoveFlickers[ind] = 0
    elif abs(diff) > moveTreshold_upper and curMoveFlickers[ind] < flickerFramesTreshold:
        diff = 0
        curMoveFlickers[ind] += 1
    elif curMoveFlickers[ind] == flickerFramesTreshold:
        curMoveFlickers[ind] = 0

    diff *= scaleFactors[ind]
    return diff

# check if the radius is correct
# return True/False
def isRadiusCorrect(radius):
    global curRadiusFlickers
    diffR = radius - prevRadius
    radiusCorrect = True
    if abs(diffR) > radiusTreshold and curRadiusFlickers < flickerFramesTreshold:
        radiusCorrect = False
        curRadiusFlickers += 1
    elif abs(diffR) > radiusTreshold and curRadiusFlickers == flickerFramesTreshold:
        radiusCorrect = True
        curRadiusFlickers = 0
    return radiusCorrect

# resets the moving control variables to default
# if the tracker target is covered and then moved and uncovered again
# this will enable similar effect to lifting and moving a mouse
def resetMovement():
    global started
    global prevCenter
    global prevRadius
    global curRadiusFlickers
    global curMoveFlickers
    started = False
    prevCenter = None
    prevRadius = -1
    curRadiusFlickers = 0
    curMoveFlickers = [0, 0]

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

    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
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
        else: 
            resetMovement()
    else:
        resetMovement()

    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF

    if key == ord("q"):
        break

vs.stop()
cv2.destroyAllWindows()