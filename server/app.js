/**
 * This module provides entry and exit point of the application.
 * It takes care of the correct setup and shutdown.
 * 
 * @module 
 * @author Luka Kralj
 * @version 1.0
 */

const ngrok = require('ngrok');
let ngrokOpts = {
    "proto": "http",
    "addr": 5637
}

console.log("Connecting ngrok...");
let ngrokUrl = undefined;
(async function () {
    ngrokUrl = await ngrok.connect(ngrokOpts);
    console.log("Ngrok connected: " + ngrokUrl);
    console.log("Ngrok using port: " + ngrokOpts.addr);
})().catch((err) => {
    console.log(err);
    console.log("Ngrok could not start.");
});

let app = require('express')();
app.listen(ngrokOpts.addr, () => {
    console.log("Server waiting...");
});


app.get("/test", (req, res) => {
    console.log(req);
    res.status(200).send({
        msg: "All good fam"
    });
});










/** Block Ctrl+C plus graceful shutdown. */
process.on('SIGINT', () => {
    onExit();
});

/** Graceful shutdown. */
process.on('SIGTERM', () => {
    onExit();
});

/**
 * Exit application.
 */
async function onExit() {
    console.log("Stopping...");
    try {
        await ngrok.disconnect();
        await ngrok.kill();
        console.log("Disconnected ngrok.");
    }
    catch (err) {
        console.log(err);
        process.exit(1);
    }
    process.exit(0);
}
