from imutils.video import VideoStream
from imutils.video import FPS
import imutils
import time
import cv2
import numpy as np

# config properties
trackerType = "csrt"

# extract the OpenCV version info
(major, minor) = cv2.__version__.split(".")[:2]

if int(major) == 3 and int(minor) < 3:
    # OpenCV 3.2 OR BEFORE --> a special factory function
    tracker = cv2.Tracker_create(trackerType.upper())
else:
    # OpenCV 3.3 OR NEWER --> explicitly call the appropriate object tracker constructor:
    trackers = {
        "csrt": cv2.TrackerCSRT_create,
        "kcf": cv2.TrackerKCF_create,
        "boosting": cv2.TrackerBoosting_create,
        "mil": cv2.TrackerMIL_create,
        "tld": cv2.TrackerTLD_create,
        "medianflow": cv2.TrackerMedianFlow_create,
        "mosse": cv2.TrackerMOSSE_create
    }

    tracker = trackers[trackerType]()

print("[INFO] starting video stream...")
vs = VideoStream(src=0).start()
time.sleep(1.0)

# FPS throughput estimator
fps = None

initBox = None

while True:
    frame = vs.read()

    # check if end of the stream
    if frame is None:
        print("[INFO] Frame is None, breaking from loop...")
        break

    # =========== process frame ================

    # faster processing
    frame = imutils.resize(frame, width=500)

    # Convert BGR to HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # define range of green color in HSV
    lower = np.array([110, 100, 100])
    upper = np.array([130, 255, 255])

    # Threshold the HSV image to get only blue colors
    mask = cv2.inRange(hsv, lower, upper)
    result = cv2.bitwise_and(frame, frame, mask=mask)

    frame = result
    (h, w) = frame.shape[:2]

    # =========== tracking ================

    if initBox is None:
        initBox = cv2.selectROI("Frame", frame, fromCenter=False,
			showCrosshair=True)
        tracker.init(frame, initBox)
        fps = FPS().start()

    if initBox is not None:
        (success, box) = tracker.update(frame)
        if success:
            (x, y, w, h) = [int(v) for v in box]
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            fps.update()
            fps.stop()

        info = [
            ("Tracker", trackerType),
            ("Success", "Yes" if success else "No"),
            ("FPS", "{:.2f}".format(fps.fps())),
        ]

        # loop over the info tuples and draw them on our frame
        for (i, (k, v)) in enumerate(info):
            text = "{}: {}".format(k, v)
            cv2.putText(frame, text, (10, h - ((i * 20) + 20)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

    # =========== display result ================
    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF

vs.stop()

# close all windows
cv2.destroyAllWindows()
