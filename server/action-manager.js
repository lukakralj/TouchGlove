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

let mouseSpeeds = [1,3,6,10];
let curSpeedInd = 1; // speed 3
let isMouseDown = false;
let isWindowSwitcherOn = false;

/**
 * 
 * @param {number} actionId Number representation of the triggers.
 */
function dispatchAction(actionId) {
    console.log("received: " + actionId);
    switch (actionId) {
        case 0b100: console.log("test: 1 on; 2,3 off"); break;
        default: console.log("no action");
    }
}

function mouseMoveDown() {
    runCmd(`xte 'mousermove 0 ${mouseSpeeds[curSpeedInd]}'`);
}

function mouseMoveUp() {
    runCmd(`xte 'mousermove 0 -${mouseSpeeds[curSpeedInd]}'`);
}

function mouseMoveLeft() {
    runCmd(`xte 'mousermove -${mouseSpeeds[curSpeedInd]} 0'`);
}

function mouseMoveRight() {
    runCmd(`xte 'mousermove ${mouseSpeeds[curSpeedInd]} 0'`);
}

function mouseMoveUpLeft() {
    runCmd(`xte 'mousermove -${mouseSpeeds[curSpeedInd]} -${mouseSpeeds[curSpeedInd]}'`);
}

function mouseMoveUpRight() {
    runCmd(`xte 'mousermove ${mouseSpeeds[curSpeedInd]} -${mouseSpeeds[curSpeedInd]}'`);
}

function mouseMoveDownLeft() {
    runCmd(`xte 'mousermove -${mouseSpeeds[curSpeedInd]} ${mouseSpeeds[curSpeedInd]}'`);
}

function mouseMoveDownRight() {
    runCmd(`xte 'mousermove ${mouseSpeeds[curSpeedInd]} ${mouseSpeeds[curSpeedInd]}'`);
}

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

function changeMouseSpeed() {
    curSpeedInd = (curSpeedInd + 1) % mouseSpeeds.length;
}

function mouseDown() {
    isMouseDown = true;
    runCmd("xte 'mousedown 1'");
}

function mouseUp() {
    isMouseDown = false;
    runCmd("xte 'mouseup 1'");
}

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