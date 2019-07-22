const gpio = require('./Gpio');
const Gpio = gpio.Gpio;

let sensLeft = undefined;
let sensRight = undefined;
let threshold = 100;
init().then(() => run());

async function init() {
    try {
        sensLeft = new Gpio(26, gpio.DIR_IN);
        sensRight = new Gpio(29, gpio.DIR_IN);
        await sensLeft.init();
        await sensRight.init();
    }
    catch (err) {
        Console.log(err);
        return false;
    }
}

async function run() {

    let val1 = undefined;
    let val2 = undefined;
    let comboStart = new Date();
    let executingAction = false;

    while (true) {
        let temp1 = sensLeft.readValue();
        let temp2 = sensRight.readValue();

        if (val1 == undefined || temp1 != val1 || temp2 != val2) {
            executingAction = false;
            comboStart = new Date();
        }

        val1 = temp1;
        val2 = temp2;

        let duration = new Date() - comboStart;
        if (!executingAction && duration >= threshold) {
            executingAction = true;
            execAction(val1, val2);
        }

        await sleep(1000);
    }


}

let dir = 1;
function execAction(val1, val2) {
	if (val1 == '1' && val2 == '1') {
		if (dir == 1) {
			cmdOutput("xte 'mousermove -3 0'");
		}
		else {
			cmdOutput("xte 'mousermove 3 0'");
		}
	}
	else if (val1 == '1') {
		cmdOutput("xte 'mousermove 0 -3'");
	}
	else if (val2 == '1') {
		cmdOutput("xte 'mousermove 0 3'");
	}
	else {
        console.log("no action-")
		dir *= -1;
	}
}

/**
 * Executes the given command and returns the response of the command.
 * 
 * @param {string} cmd One of the constants above.
 * @param {number} timeout Number of milliseconds.
 * @returns {string} Comand output or undefined if an error occurred.
 */
async function cmdOutput(cmd, timeout = 10000) {
    let output = undefined;
    let finished = false;
    const proc = exec(cmd, (err, stdout, stderr) => {
        if (err) {
            console.log(err)
            console.log(stderr);
        }
        else {
            if (stdout === undefined || stdout.trim().length == 0) {
                // some commands might have empty output
                stdout = true;
            }
            output = stdout;
        }
        finished = true;
    });

    let time = 0;
    while (!finished && time < timeout) {
        time++;
        await sleep(1);
    }
    return output;
}





/** Block Ctrl+C plus graceful shutdown. */
process.on('SIGINT', async () => {
    onExit();
});

/** Graceful shutdown. */
process.on('SIGTERM', async () => {
    onExit();
});

function onExit() {
    if (sensLeft != undefined) {
        sensLeft.unexport();
    }
    if (sensRight != undefined) {
        sensRight.unexport();
    }
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