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
int threshold_ms = 100; // 1 second

void execAction(const string, const string);
void onStop(int sig);

int main() {
	signal(SIGTERM, onStop);
	signal(SIGINT, onStop);

	string val1 = "";
	string val2 = "";
	auto comboStart = high_resolution_clock::now();
	bool executingAction = false;
	string temp1 = "";
	string temp2 = "";

	while (true) {
		temp1 = sensLeft.readValue();
		temp2 = sensRight.readValue();

		if (val1 == "" || temp1 != val1 || temp2 != val2) {
			executingAction = false;
			comboStart = high_resolution_clock::now();
		}

		val1 = temp1;
		val2 = temp2;

		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - comboStart); 

		if (!executingAction && duration.count() >= threshold_ms) {
			executingAction = true;
			execAction(val1, val2);
		}

		usleep(100); // microseconds!!!
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
void execAction(const string val1, const string val2) {
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
