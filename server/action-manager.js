/**
 * This module assigns actions to specific action IDs.
 * 
 * @module 
 * @author Luka Kralj
 * @version 1.0
 */

module.exports = {
    dispatchAction,
}

const exec = require('child_process').exec;

let mouseSpeeds = [1,2,3,6,10];
let curSpeedInd = 2; // speed 3

/** Sets the speed that the python script can then use. */
process.env.MOUSE_SPEED = mouseSpeeds[curSpeedInd];

//let isMouseDown = false;
let isWindowSwitcherOn = false;
const indexF_mask = 0b100;
const middleF_mask = 0b010;
const ringF_mask = 0b001;

console.log("Starting hand tracker...");
runCmd("python hand-tracker.py");

/**
 * 
 * @param {number} actionId Number representation of the triggers.
 */
function dispatchAction(actionId) {
    console.log("received: " + actionId);

    // TODO: probably needs some refactoring
    if ((actionId & indexF_mask) != 0) {
        startIndexF();
    }
    else {
        endIndexF();
    }

    if ((actionId & middleF_mask) != 0) {
        startMiddleF();
    }
    else {
        endMiddleF();
    }

    if ((actionId & ringF_mask) != 0) {
        startRingF();
    }
    else {
        endRingF();
    }

}

// Control variable for action dispatching
let indexF_waitForEnd = false;
let middleF_waitForEnd = false;
let ringF_waitForEnd = false;

//============================
// Functions below handle what happens at the start and what at the end
// of the touch on sensor
//============================
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

function endIndexF() {
    if (!indexF_waitForEnd) {
        return;
    }

    if (!isWindowSwitcherOn) {
        mouseUp();
    }
    indexF_waitForEnd = false;
}


function startMiddleF() {
    if (middleF_waitForEnd) {
        return;
    }
    toggleWindowSwitcher();
    middleF_waitForEnd = true;
}

function endMiddleF() {
    if (!middleF_waitForEnd) {
        return;
    }
    toggleWindowSwitcher();
    middleF_waitForEnd = false;
}

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

function endRingF() {
    if (!ringF_waitForEnd) {
        return;
    }
    ringF_waitForEnd = false;
}

//============================
// Helper functions below actually 
// execute mouse movements etc.
//============================

/**
 * Turn on/off scrolling through the active windows.
 */
function toggleWindowSwitcher() {
    if (isWindowSwitcherOn) {
        isWindowSwitcherOn = false;
        runCmd("xte 'keydown Alt_L'");
    }
    else {
        isWindowSwitcherOn = true;
        runCmd("xte 'keyup Alt_L'");
    }
}

/**
 * Move to the active window above the currently selected one.
 */
function toPrevWindow() {
    runCmd("xte 'key Tab'");
}

/**
 * Move to the active window below the currently selected one.
 */
function toNextWindow() {
    runCmd("xte 'keydown Shift_L'");
    runCmd("xte 'key Tab'");
    runCmd("xte 'keyup Shift_L'");
}

/**
 * Change the speed of the mouse.
 */
function changeMouseSpeed() {
    curSpeedInd = (curSpeedInd + 1) % mouseSpeeds.length;
    process.env.MOUSE_SPEED = mouseSpeeds[curSpeedInd];
}

/**
 * Left mouse button is pressed.
 */
function mouseDown() {
    //isMouseDown = true;
    runCmd("xte 'mousedown 1'");
}

/**
 * Left mouse button is released.
 */
function mouseUp() {
    //isMouseDown = false;
    runCmd("xte 'mouseup 1'");
}

/**
 * Executes the given command and prints the output of the command, if any,
 * to stdout of this script.
 * 
 * @param {string} cmd Command we want to run.
 */
function runCmd(cmd) {
    exec(cmd, (err, stdout, stderr) => {
        if (err || stderr) {
            console.log("======Error with command '" + cmd + "' ======");
            console.log(err);
            console.log(stderr);
            console.log("=====end output: '" + cmd + "' ========");
        }
        if (stdout) {
            console.log("Stdout for command: '" + cmd + "': " + stdout);
        }
    });
}