/**
 * This module provides entry and exit point of the server.
 * It takes care of the correct setup and shutdown.
 * 
 * @module 
 * @author Luka Kralj
 * @version 1.0
 */

const fs = require("fs");
const actionManager = require("./action-manager");

const stream = fs.createReadStream("/dev/ttyACM0");

stream.on("data", function(data) {
    let chunk = data.toString().trim();
    if (chunk.length == 0) {
        return;
    }
    encoding = Number(chunk)
    actionManager.dispatchAction(encoding);
});


