/**
 * This module provides entry and exit point of the application.
 * It takes care of the correct setup and shutdown.
 * 
 * @module 
 * @author Luka Kralj
 * @version 1.0
 */

const port = 5637;

const actionManager = require('./action-manager');
const app = require('express')();
app.listen(port, () => {
    console.log("Server waiting...");
});

app.get('/sens/:actionId', (req, res) => {
    actionManager.dispatchAction(parseInt(req.params.actionId));
    return res.status(200).send();
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
        await ngrok.kill();
        console.log("Disconnected ngrok.");
    }
    catch (err) {
        console.log(err);
        process.exit(1);
    }
    console.log("Exiting...");
    process.exit(0);
}
