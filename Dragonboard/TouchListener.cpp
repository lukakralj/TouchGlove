#include <iostream>
#include <string>
#include <vector>
#include "Gpio.h"
#include <unistd.h>
#include <chrono> 
#include <signal.h>

// Uncomment the bottom line to disable assertions.
//#define NDEBUG 
#include <assert.h>

using namespace std::chrono; 
using namespace std;

int threshold_ms = 25;

int encodeAction(const int[], int);
void triggerAction(const int);
void onStop(int sig);
void testEncodeAction();

// Initialise all the sensors
int numOfSensors = 2;
Gpio sensors[] = {
	Gpio(26,"in"), 
	Gpio(29, "in") };

// Set up all the control variables
auto now = high_resolution_clock::now();
time_point<std::chrono::_V2::system_clock, std::chrono::nanoseconds> changeTimers[] = {
	now, now
};
int curAction[] = { -1, -1 };
int candidates[] = { -1, -1 };
int lastRead[] = { -1, -1 };

int main() {
	signal(SIGTERM, onStop);
	signal(SIGINT, onStop);

	assert (numOfSensors == (sizeof(sensors)/sizeof(sensors[0])));
	assert (numOfSensors == (sizeof(changeTimers)/sizeof(changeTimers[0])));
	assert (numOfSensors == (sizeof(curAction)/sizeof(curAction[0])));
	assert (numOfSensors == (sizeof(candidates)/sizeof(candidates[0])));
	assert (numOfSensors == (sizeof(lastRead)/sizeof(lastRead)));
	testEncodeAction();

	cout << "Setup complete. Listening..." << endl;
	while (true) {
		for (int i = 0; i < numOfSensors; ++i) {
			lastRead[i] = sensors[i].readValue();

			if (curAction[i] == -1) {
				// Happens only once at the beginning
				curAction[i] == lastRead[i];
			}
			else {
				// A change might have occurred - start timing the change
				if (lastRead[i] != candidates[i]) {
					candidates[i] = lastRead[i];
					changeTimers[i] = high_resolution_clock::now();
				}// else candidate value did not change
			}

			if (candidates[i] != curAction[i]) {
				// A different action is wanted - check for threshold.
				auto timeSinceLastChange = duration_cast<milliseconds>(high_resolution_clock::now() - changeTimers[i]);
				if (timeSinceLastChange.count() >= threshold_ms) {
					curAction[i] = candidates[i];
				}
			}
		}

		triggerAction(encodeAction(curAction, numOfSensors));

		usleep(1000); // 1 ms (param in microseconds!!!)
	}

	return 0;
}

void onStop(int sig) {
	cout << "Intercepted sig: " << sig << endl;
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
int encodeAction(const int sensorValues[], int length) {
	int encoded = 0;
	int multiplier = 1;
	for (int i = length - 1; i >= 0; --i) {
		encoded += multiplier * sensorValues[i];
		multiplier *= 10;
	}
	return encoded;
}

void testEncodeAction() {
	int hundred[] = {1,0,0};
	assert (encodeAction(hundred, 3) == 100);
	int thousand[] = {1,0,0,0};
	assert (encodeAction(thousand, 4) == 1000);
	int oneOhOne[] = {1,0,1};
	assert (encodeAction(oneOhOne, 3) == 101);
	int one[] = {1};
	assert (encodeAction(one, 1) == 1);
	int zero[] = {};
	assert (encodeAction(zero, 0) == 0);
	int rand[] = {1,0,0,1,1,1,0};
	assert (encodeAction(rand, 7) == 1001110);
}

void triggerAction(const int encoding) {
	cout << "Triggered: " << encoding << endl;
}