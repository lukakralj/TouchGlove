'''
This is the script that is responsible for moving the mouse around.
It is tracking the tracker target. This target needs to be of a 
distinct colour compared to the background.

This script can be run as a standalone.

Author: Luka Kralj

I took some bits of code from this site: https://www.pyimagesearch.com/2015/09/14/ball-tracking-with-opencv/
'''

from imutils.video import VideoStream
import numpy as np
import cv2
import imutils
import time
import subprocess
import os
import sys
import signal
import threading

# Tracker target colour range
lower = np.array([75, 100, 100]) #green
upper = np.array([95, 255, 255])

# True if you want to see the video display of the tracker, False otherwise.
displayTracker = False 
# Size of the frame to run the calculations on.
frameW = 600
frameH = -1 # calculated below
moveThreshold_lower = 1 # pixels
moveThreshold_upper = 400
radiusThreshold = 5 #pixels; move ignored if radius changes for more than this (to prevent flickering)
flickerFramesThreshold = 5 # number of frames in which the big increase in radius or move is ignored
screenBorder = 40 # pixels; the area of the image that represents the screen

mouseSpeed = 3 # default value

# Get screen size.
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

# Used to covert a move length from the frame to the screen.
XscaleFactor = (float(screenW) / float(frameW)) * -1 # multiply by -1 to change direction because of mirroring
YscaleFactor = float(screenH) / float(frameH)
scaleFactors = (XscaleFactor, YscaleFactor)

# Control variables
started = False
prevCenter = None
prevRadius = -1
curRadiusFlickers = 0
curMoveFlickers = [0, 0]

'''
Decide if the move is to be executed.
'''
def processMove(center, radius):
    if not started: 
        return

    global prevCenter
    global prevRadius

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
            os.system(cmd) # execute mouse move
            prevCenter = center
            prevRadius = radius

''' 
Calculate the difference of the coordinate compared to the previous frame.

Params: Use ind = 0 for X-coordinate; and ind = 1 for Y-coordinate.

Returns: float - The scaled difference (according to screen scale factors).
'''
def calculateDiff(center, ind):
    global curMoveFlickers

    diff = float(center[ind]) - float(prevCenter[ind])
    if abs(diff) < moveThreshold_lower:
        diff = 0
        curMoveFlickers[ind] = 0
    elif abs(diff) > moveThreshold_upper and curMoveFlickers[ind] < flickerFramesThreshold:
        diff = 0
        curMoveFlickers[ind] += 1
    elif curMoveFlickers[ind] == flickerFramesThreshold:
        curMoveFlickers[ind] = 0

    diff *= scaleFactors[ind] * float(mouseSpeed)/2
    return diff

''' 
Check if the radius is correct; prevents flickers in radius.
Returns: True if radius is correct, False if the move should be ignored.
'''
def isRadiusCorrect(radius):
    global curRadiusFlickers

    diffR = radius - prevRadius
    radiusCorrect = True
    if abs(diffR) > radiusThreshold and curRadiusFlickers < flickerFramesThreshold:
        radiusCorrect = False
        curRadiusFlickers += 1
    elif abs(diffR) > radiusThreshold and curRadiusFlickers == flickerFramesThreshold:
        radiusCorrect = True
        curRadiusFlickers = 0
    return radiusCorrect

'''
Resets the control variables to default.
If the tracker target is covered and then moved and uncovered again
this will enable similar effect to lifting and moving a mouse.
'''
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

stopStdinThread = False
'''
Listen for stdin input on a separate thread.
If an integer value is read from stdin the mouse
speed will change. Otherwise nothing happens.
'''
def processStdin():
    global mouseSpeed
    while not stopStdinThread:
        line = sys.stdin.readline().strip()
        if line.isdigit():
            mouseSpeed = int(line)

stdinThread = threading.Thread(target=processStdin)
stdinThread.start()

vs = VideoStream(src=0).start()
time.sleep(2.0)

'''
Called before the program exits. Takes care of cleaning up.
'''
def onExit(signum, frame):
    global stopStdinThread
    stopStdinThread = True
    vs.stop()
    cv2.destroyAllWindows()
    stdinThread.join() # Need to press Enter to exit if running a script as a standalone.
    sys.exit(0)
    
signal.signal(signal.SIGINT, onExit)
signal.signal(signal.SIGTERM, onExit)

while True:
    frame = vs.read()

    if frame is None:
        break

    # Resize the frame, blur it, and convert it to the HSV color space.
    frame = imutils.resize(frame, width=frameW, height=frameH)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # Construct a mask for the selected colour range, then perform
    # a series of dilations and erosions to remove any small
    # blobs left in the mask.
    mask = cv2.inRange(hsv, lower, upper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None

    # Only proceed if at least one contour was found.
    if len(cnts) > 0:
        # Find the largest contour in the mask, then use it to compute the minimum enclosing circle and centroid.
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

        # Only proceed if the radius meets a minimum size.
        if radius > 10:
            started = True
            # Draw the circle and centroid on the frame.
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)
            processMove((x,y), radius)
        else: 
            resetMovement()
    else:
        resetMovement()

    if displayTracker:
        text = "mouse speed: {}".format(mouseSpeed)
        cv2.putText(frame, text, (10, frameH - (40)), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
        cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF

    if key == ord("q"):
        break

onExit(2, None)
