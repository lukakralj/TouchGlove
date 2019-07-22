#include <iostream>
#include <string>
#include <vector>
#include "GpioProcessor.h"
#include <unistd.h>
#include <chrono> 
using namespace std::chrono; 
using namespace std;

GpioProcessor *gpioProcessor = nullptr;
Gpio *sensor1 = nullptr;
Gpio *sensor2 = nullptr;
int treshold_ms = 100; // 1 second


void execAction(const char*, const char*);

int main() {
	gpioProcessor = new GpioProcessor();

	sensor1 = gpioProcessor->getPin26();
	sensor2 = gpioProcessor->getPin29();

	sensor1->setIn();
	sensor2->setIn();

	char* val1 = nullptr;
	char* val2 = nullptr;
	auto comboStart = high_resolution_clock::now();
	bool executingAction = false;

	while (true) {
		char* temp1 = sensor1->getValue();
		char* temp2 = sensor2->getValue();

		if (val1 == nullptr || *temp1 != *val1 || *temp2 != *val2) {
			executingAction = false;
			comboStart = high_resolution_clock::now();
		}

		val1 = temp1;
		val2 = temp2;

		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - comboStart); 

		if (!executingAction && duration.count() >= treshold_ms) {
			//executingAction = true;
			execAction(val1, val2);
		}

		usleep(100); // microseconds!!!
	}

	gpioProcessor->cleanPins();
	delete (gpioProcessor);
	delete (sensor1);
	delete (sensor2);
	return 0;
}

int dir = 1;
void execAction(const char* val1, const char* val2) {
	if (*val1 == '1' && *val2 == '1') {
		if (dir == 1) {
			system("xte 'mousermove -3 0'");
		}
		else {
			system("xte 'mousermove 3 0'");
		}
	}
	else if (*val1 == '1') {
		system("xte 'mousermove 0 -3'");
	}
	else if (*val2 == '1') {
		system("xte 'mousermove 0 3'");
	}
	else {
		dir *= -1;
	}
}