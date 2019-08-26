/**
 * This module provides entry point of the laptop app.
 * It parses data received via USB from the Dragonboard
 * and sends it to the action-manager module.
 * 
 * @module app.js
 * @author Luka Kralj
 * @version 1.0
 */

const fs = require("fs");

// Import triggers the action manager setup.
const actionManager = require("./action-manager");

/**
 * N.B. If more than one device is connected this might 
 * be different. It should be something like /dev/ttyAC*, regardless.
 */
const stream = fs.createReadStream("/dev/ttyACM0");

stream.on("data", function(data) {
    let chunk = data.toString().trim();
    if (chunk.length == 0) {
        return;
    }
    // Dragonboard is only sending integer encodings.
    encoding = Number(chunk)
    actionManager.dispatchAction(encoding);
});


