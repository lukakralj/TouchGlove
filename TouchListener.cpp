#include <iostream>
#include <string>
#include <vector>
#include "Gpio.h"
#include <unistd.h>
#include <chrono> 
#include <signal.h>

using namespace std::chrono; 
using namespace std;

Gpio sensLeft(26, "in");
Gpio sensRight(29, "in");
int threshold_ms = 100; // 0.1 second

void applyAction(const string, const string);
void onStop(int sig);

int main() {
	signal(SIGTERM, onStop);
	signal(SIGINT, onStop);

	string curActionVal1 = "";
	string curActionVal2 = "";

	string candidateVal1 = "";
	string candidateVal2 = "";

	auto changeStartTime = high_resolution_clock::now();
	
	string lastRead1 = "";
	string lastRead2 = "";
	
	//bool executingAction = false;
	

	while (true) {
		lastRead1 = sensLeft.readValue();
		lastRead2 = sensRight.readValue();

		if (curActionVal1 == "") {
			// Happens only once at the beginning
			curActionVal1 = lastRead1;
			curActionVal2 = lastRead2;
		}
		else if (lastRead1 != candidateVal1 || lastRead2 != candidateVal2) {
			// A change occurred - start timing the change
			candidateVal1 = lastRead1;
			candidateVal2 = lastRead2;
			changeStartTime = high_resolution_clock::now();
		} // else candidate value did not change

		if (candidateVal1 != curActionVal1 || candidateVal2 != curActionVal2) {
			// A different action is wanted - check for threshold.
			auto timeSinceLastChange = duration_cast<milliseconds>(high_resolution_clock::now() - changeStartTime);
			if (timeSinceLastChange.count() >= threshold_ms) {
				curActionVal1 = candidateVal1;
				curActionVal2 = candidateVal2;
			}
		}

		applyAction(curActionVal1, curActionVal2);

		usleep(1000); // 1 ms (param in microseconds!!!)
	}

	return 0;
}

void onStop(int sig) {
	cout << "Intercepted sig: " << sig << endl;
	sensLeft.unexportPin();
	sensRight.unexportPin();
	cout << "Pins exported." << endl;
	exit(0);
}

int dir = 1;
void applyAction(const string val1, const string val2) {
	if (val1 == "1" && val2 == "1") {
		if (dir == 1) {
			cout << "move left" << endl;
			//system("xte 'mousermove -3 0'");
		}
		else {
			cout << "move right" << endl;
			//system("xte 'mousermove 3 0'");
		}
	}
	else if (val1 == "1") {
		cout << "move up" << endl;
		//system("xte 'mousermove 0 -3'");
	}
	else if (val2 == "1") {
		cout << "move down" << endl;
		//system("xte 'mousermove 0 3'");
	}
	else {
		cout << "no action" << endl;
		dir *= -1;
	}
}
