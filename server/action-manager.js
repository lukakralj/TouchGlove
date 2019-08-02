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