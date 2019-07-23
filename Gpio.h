#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#define HIGH "1"
#define LOW "0"
#define OUT "out"
#define IN "in"

#define PATH_EXPORT "/sys/class/gpio/export"
#define PATH_UNEXPORT "/sys/class/gpio/unexport"

using namespace std;

string exec(const string);
int convertPhysicalPin(int);

class Gpio {

	private:
		string pinNo;
		string dir;

		string getDirPath();
		string getValuePath();
		void exportPin();

	public:
		Gpio(int, string);
		string getDirection();
		string readValue();
		void setHigh();
		void setLow();
		void unexportPin();
};

struct InvalidDirectionException : public exception {
   const char * what () const throw () {
      return "Unknown direction set to the pin.";
   }
};

struct InvalidPhysicalPinException : public exception {
   const char * what () const throw () {
      return "Invalid pin number - could not be exported.";
   }
};

struct InvalidOperationException : public exception {
   const char * what () const throw () {
      return "Invalid operation for this pin type.";
   }
};

/*
	 * gets the pin defined by the integer. This number does not always
	 * correspond with the pin number: For example, on the IFC6410, GPIO pin 21
	 * corresponds to the operating system pin number 6.
	 */
Gpio::Gpio(int physicalPin, string direction) {
	pinNo = to_string(convertPhysicalPin(physicalPin));
	if (direction != OUT && direction != IN) {
		throw InvalidDirectionException();
	}
	dir = direction;
	
	exportPin();
	// set direction
	string cmd = "echo " + dir + " > " + getDirPath();
	exec(cmd);
}

/*Get pin direction.
		in -> Input.
		out -> Output.
	*/
string Gpio::getDirection() {
	string cmd = "cat " + getDirPath();
	return exec(cmd);
}

/*Get pin value.
		0 -> Low Level.
		1 -> High Level
	*/
string Gpio::readValue() {
	string cmd = "cat " + getValuePath();
	return exec(cmd); 
}

/* sets pin high */
void Gpio::setHigh() {
	if (dir == IN) {
		throw InvalidOperationException();
	}
	string cmd = "echo 1 > " + getValuePath();
	exec(cmd); 
}

/* sets pin low */
void Gpio::setLow() {
	if (dir == IN) {
		throw InvalidOperationException();
	}
	string cmd = "echo 0 > " + getValuePath();
	exec(cmd); 
}

string Gpio::getDirPath() {
	return "/sys/class/gpio/gpio" + pinNo + "/direction";
}

string Gpio::getValuePath() {
	return "/sys/class/gpio/gpio" + pinNo + "/value";
}

void Gpio::exportPin() {
	string cmd = "echo " + pinNo + " > /sys/class/gpio/export";
	exec(cmd);
}

void Gpio::unexportPin() {
	string cmd = "echo " + pinNo + " > /sys/class/gpio/unexport";
	exec(cmd);
}

int convertPhysicalPin(int pin) {
    switch (pin) {
        case 24: return 12; // B
        case 25: return 13; // C
        case 26: return 69; // D 
        case 27: return 115; // E
        case 28: return 4; // F
        case 29: return 24; // G 
        case 30: return 25; // H 
        case 31: return 35; // I
        case 32: return 34; // J
        case 33: return 28; // K
        case 34: return 33; // L
        default: throw InvalidPhysicalPinException();
    }
}

/**
 * Execute the command and return its stdout.
 */
string exec(const string cmd) {
	char cstr[cmd.size() + 1];
	strcpy(cstr, cmd.c_str());

	array<char, 128> buffer;
	string result;
	unique_ptr<FILE, decltype(&pclose)> pipe(popen(cstr, "r"), pclose);
	if (!pipe)
	{
		throw runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}

	return result;
}