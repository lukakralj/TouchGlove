/**
 * This module triggers appropriate actions according to the encoding.
 * 
 * @module action-manager.js
 * @author Luka Kralj
 * @version 1.0
 */

module.exports = {
    dispatchAction
}

const exec = require('child_process').exec;

/**
 * Username of the user running the app.
 * Since the app is running as root the notifications
 * will not be showing on the screen. However, the root can execute
 * the notifying as a different user so that the notifications will be showing
 * on the actual user.
 */
const username = "lukakralj";

// Possible mouse speeds (can be added or removed without a change in functionality.).
let mouseSpeeds = [1,2,3,6,10];
let curSpeedInd = 2; // speed 3

// Control variables.
let isMouseDown = false;
let isWindowSwitcherOn = false;
const indexF_mask = 0b100;
const middleF_mask = 0b010;
const ringF_mask = 0b001;

console.log("Starting hand tracker...");
// Initialise hand tracker and get the reference to its process.
const trackerProcess = runCmd("python hand-tracker.py");
console.log("Hand tracker has started.");

/** Block Ctrl+C plus graceful shutdown. */
process.on('SIGINT', () => {
    onExit();
});

/** Graceful shutdown. */
process.on('SIGTERM', () => {
    onExit();
});

/**
 * Triggers actions for certain fingers according to the encoding
 * sent by the Dragonboard.
 * 
 * @param {number} encoding Number representation of the triggers.
 */
function dispatchAction(encoding) {
    // TODO: probably needs some refactoring
    if ((encoding & indexF_mask) != 0) {
        startIndexF();
    }
    else {
        endIndexF();
    }

    if ((encoding & middleF_mask) != 0) {
        startMiddleF();
    }
    else {
        endMiddleF();
    }

    if ((encoding & ringF_mask) != 0) {
        startRingF();
    }
    else {
        endRingF();
    }
}

// Control variable for action dispatching.
let indexF_waitForEnd = false;
let middleF_waitForEnd = false;
let ringF_waitForEnd = false;

//============================
// Functions below handle what happens at the start and what at the end
// of the touch on a sensor. Aka it reacts to pressed and released actions
// on the sensor.
//============================

/** When index finger sensor is pressed. */
function startIndexF() {
    if (indexF_waitForEnd) {
        return;
    }

    if (isWindowSwitcherOn) {
        toPrevWindow();
    }
    else {
        mouseDown();
    }
    indexF_waitForEnd = true;
}

/** When index finger sensor is released. */
function endIndexF() {
    if (!indexF_waitForEnd) {
        return;
    }

    if (!isWindowSwitcherOn) {
        mouseUp();
    }
    indexF_waitForEnd = false;
}

/** When middle finger sensor is pressed. */
function startMiddleF() {
    if (middleF_waitForEnd) {
        return;
    }
    toggleWindowSwitcher();
    middleF_waitForEnd = true;
}

/** When middle finger sensor is released. */
function endMiddleF() {
    if (!middleF_waitForEnd) {
        return;
    }
    middleF_waitForEnd = false;
}

/** When ring finger sensor is pressed. */
function startRingF() {
    if (ringF_waitForEnd) {
        return;
    }

    if (isWindowSwitcherOn) {
        toNextWindow();
    }
    else {
        changeMouseSpeed();
    }
    ringF_waitForEnd = true;
}

/** When ring finger sensor is released. */
function endRingF() {
    if (!ringF_waitForEnd) {
        return;
    }
    ringF_waitForEnd = false;
}

//============================
// Below are the functions that actually execute the
// commands to simulate mouse actions.
//============================

/**
 * Turn on/off scrolling through the active windows.
 */
function toggleWindowSwitcher() {
    isWindowSwitcherOn = !isWindowSwitcherOn;
    // Notify the user that the window switching mode changed.
    runCmd("su " + username + " -c \"notify-send 'WINDOW SWITCHING MODE: " + (isWindowSwitcherOn ? "ON" : "OFF") + "' -t 1000\"");
}

/**
 * Move to the last active window.
 */
function toPrevWindow() {
    runCmd("xte 'keydown Alt_L' 'key Tab' 'keyup Alt_L'");
}

/**
 * Move to the next window.
 */
function toNextWindow() {
    runCmd("xte 'keydown Alt_L' 'keydown Shift_L' 'key Tab' 'keyup Shift_L' 'keyup Alt_L'");
}

/**
 * Change the speed of the mouse.
 */
function changeMouseSpeed() {
    curSpeedInd = (curSpeedInd + 1) % mouseSpeeds.length;
    // Write to stdin of the tracker script that is listening for speed changes.
    trackerProcess.stdin.write(mouseSpeeds[curSpeedInd].toString() + " \n")
    // notify the user that the mouse speed changed
    runCmd("su " + username + " -c \"notify-send 'MOUSE SPEED CHANGED TO: " + mouseSpeeds[curSpeedInd] + "' -t 1000\"");
}

/**
 * Left mouse button is pressed.
 */
function mouseDown() {
    isMouseDown = true;
    runCmd("xte 'mousedown 1'");
}

/**
 * Left mouse button is released.
 */
function mouseUp() {
    isMouseDown = false;
    runCmd("xte 'mouseup 1'");
}

/**
 * Executes the given command and prints the output of the command, if any,
 * to stdout of this script.
 * 
 * @param {string} cmd Command we want to run.
 * @returns {child-process} Reference to the process of the command.
 */
function runCmd(cmd) {
    return exec(cmd, (err, stdout, stderr) => {
        if (err || stderr) {
            console.log("======Error with command '" + cmd + "' ======");
            //console.log(err);
            console.log(stderr);
            console.log("=====end output: '" + cmd + "' ========");
        }
        if (stdout) {
            console.log("Stdout for command: '" + cmd + "': " + stdout);
        }
    });
}

/**
 * Called before the program exits. Takes care of cleaning up.
 */
async function onExit() {
    console.log("Resetting mouse and keys...");
    if (isWindowSwitcherOn) {
        toggleWindowSwitcher();
    }
    if (isMouseDown) {
        mouseUp();
    }
    await sleep(1000);
    console.log("Ending tracker process...");
    trackerProcess.stdin.write(" \n");
    console.log("Exiting... Press Ctrl+C again.");
    process.exit(0);
}

/**
 * Await for this function to pause execution for a certain time.
 *
 * @param {number} ms Time in milliseconds
 * @returns {Promise}
 */
function sleep(ms) {
    return new Promise(resolve => {
        setTimeout(resolve, ms);
    });
}
