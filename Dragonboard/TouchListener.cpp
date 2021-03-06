/**
 * This script is listening to the sensors on the glove.
 * When they are triggered the combination is encoded and the
 * send via the USB cable to the laptop.
 * 
 * @author Luka Kralj
 * @version 1.0
 */

#include <iostream>
#include <string>
#include <vector>
#include "Gpio.h"
#include <unistd.h>
#include <chrono> 
#include <signal.h>
#include <stdlib.h>

// Uncomment the bottom line to disable assertions.
//#define NDEBUG 
#include <assert.h>

using namespace std::chrono; 
using namespace std;

int threshold_ms = 25;

unsigned int encodeAction(const int[], int);
void triggerAction(const int);
void onExit(int sig);
void testEncodeAction();

// Initialise all the sensors
int numOfSensors = 3;
Gpio sensors[] = {
	Gpio(26,"in"),  // Index finger sensor.
	Gpio(29, "in"), // Middle finger sensor.
	Gpio(34, "in")  // Ring finger sensor.
};

/** 
 * Specifies the state of sensors in which all the triggers are considered to be off.
 * Must be in binary.
 * (This enables to implement a sensor that is triggered only when it is released.)
 */
unsigned int configurationMask = 0b000;

// Set up all the control variables.
auto now = high_resolution_clock::now();
time_point<std::chrono::_V2::system_clock, std::chrono::nanoseconds> changeTimers[] = {
	now, now, now
};
int curAction[] = { -1, -1, -1 };
int candidates[] = { -1, -1, -1 };
int lastRead[] = { -1, -1, -1 };

int lastEncoding = -1; // Only send the encoding once it changes.

int main(int argc, char** argv) {
	signal(SIGTERM, onExit);
	signal(SIGINT, onExit);

	// Verify that all the control variables were set up properly.
	assert (numOfSensors == (sizeof(sensors)/sizeof(sensors[0])));
	assert (numOfSensors == (sizeof(changeTimers)/sizeof(changeTimers[0])));
	assert (numOfSensors == (sizeof(curAction)/sizeof(curAction[0])));
	assert (numOfSensors == (sizeof(candidates)/sizeof(candidates[0])));
	assert (numOfSensors == (sizeof(lastRead)/sizeof(lastRead[0])));
#if !defined(NDEBUG)	
	testEncodeAction();
#endif

	cout << "Setup complete. Listening..." << endl;
	while (true) {
		for (int i = 0; i < numOfSensors; ++i) {
			lastRead[i] = sensors[i].readValue();

			if (curAction[i] == -1) {
				// Happens only once at the beginning.
				curAction[i] = lastRead[i];
			}
			else {
				// A change might have occurred - start timing the change.
				if (lastRead[i] != candidates[i]) {
					candidates[i] = lastRead[i];
					changeTimers[i] = high_resolution_clock::now();
				} // Else the candidate value did not change.
			}

			if (candidates[i] != curAction[i]) {
				// A different action is wanted - check for threshold.
				auto timeSinceLastChange = duration_cast<milliseconds>(high_resolution_clock::now() - changeTimers[i]);
				if (timeSinceLastChange.count() >= threshold_ms) {
					curAction[i] = candidates[i];
				}
			}
		}

		// Encode and trigger the action.
		triggerAction(encodeAction(curAction, numOfSensors));

		usleep(1000); // 1 ms (param in microseconds!!!)
	}
	return 0;
}

/**
 * Called before the program exits. Takes care of cleaning up.
 */
void onExit(int sig) {
	cout << "Intercepted signal: " << sig << endl;
	for (int i = 0; i < numOfSensors; ++i) {
		sensors[i].unexportPin();
	}
	cout << "Pins exported." << endl;
	cout << "Exiting..." << endl;
	exit(0);
}

/**
 * Combines all values into an integer that uniquely represents
 * a current touch state.
 * 
 * Example:
 * 100 means that only the first sensor is triggered
 * 101 means that the first and the third sensors are triggered at the same time.
 * 
 * @param sensorValues Array of the values that were read from the sensors.
 * @param length Length of the array.
 * @return Number describing the current state of the sensors.
 */
unsigned int encodeAction(const int sensorValues[], int length) {
	unsigned int encoded = 0;

	for (int i = 0; i < length; ++i) {
		encoded <<= 1;
		encoded += (unsigned int) sensorValues[i];
	}

	encoded ^= configurationMask; // XOR
	return encoded;
}

/**
 * If the encoding changed, it is sent to to the laptop via USB.
 * 
 * @param encoding The sensor combination as returned by encodeAction().
 */
void triggerAction(const int encoding) {
	if (lastEncoding == encoding) {
		// Triggers did not change, no need to trigger action again
		return;
	}
	lastEncoding = encoding;
	string cmd = "echo " + to_string(encoding) + " > /dev/ttyGS0";
	system(cmd.c_str());
}

/**
 * Test encoding.
 * Disable by uncommenting line :
 * 		#define NDEBUG
 */
void testEncodeAction() {
	unsigned int originalMask = configurationMask;

	configurationMask = 0b0;
	int hundred[] = {1,0,0};
	assert (encodeAction(hundred, 3) == 0b100);
	int thousand[] = {1,0,0,0};
	assert (encodeAction(thousand, 4) == 0b1000);
	int oneOhOne[] = {1,0,1};
	assert (encodeAction(oneOhOne, 3) == 0b101);
	int one[] = {1};
	assert (encodeAction(one, 1) == 1);
	int zero[] = {};
	assert (encodeAction(zero, 0) == 0);
	int rand[] = {1,0,0,1,1,1,0};
	assert (encodeAction(rand, 7) == 0b1001110);

	// Test with mask = 0110
	configurationMask = 0b0110;
	int test1[] = {1,0,0,0};
	assert (encodeAction(test1, 4) == 0b1110);
	int test2[] = {0,0,0,0};
	assert (encodeAction(test2, 4) == 0b0110);
	int test3[] = {0,1,1,0};
	assert (encodeAction(test3, 4) == 0b0000);
	int test4[] = {1,0,1,0};
	assert (encodeAction(test4, 4) == 0b1100);
	int test5[] = {1,0,0,1};
	assert (encodeAction(test5, 4) == 0b1111);
	int test6[] = {0,1,0,1};
	assert (encodeAction(test6, 4) == 0b0011);

	configurationMask = originalMask;
}
