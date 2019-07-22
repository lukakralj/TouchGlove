#include <iostream>
#include <string>
#include <chrono> 
using namespace std::chrono; 

using namespace std;

int main() {

	//cout << "start" << endl;
	//auto start = high_resolution_clock::now(); 

	/*while (true) {
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start); 

		if (duration.count() >= 1000) {
			start = high_resolution_clock::now(); 
			cout << duration.count() << endl;
		}
	}*/

	system("xte 'mousermove 100 100'");

	return 0;
}
