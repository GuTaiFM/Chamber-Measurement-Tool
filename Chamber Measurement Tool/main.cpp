#include "AnalyserObj.h"
#include "SerialRotatorObj.h"
#include "SerialRotatorException.h"

int main() {
	try {
		AnalyserObj<double> analyser(400e6, 3e9, 10, 5e3, 1601, MLOG, S21, REAL32, "192.168.20.200", 23);
		
		SerialRotatorObj serRot(1, 255, 3, 4, 9600);
		serRot.rotateTo(90);
	}
	catch (boost::system::system_error &e) {
		std::cerr << "Yeah, something went derp whilst trying to connect to the analyser" << std::endl;
		std::cerr << e.what();
	}
	catch (SerialRotatorException &e) {
		std::cerr << "Something went wrong with the rotator" << std::endl;
		std::cerr << e.what() << std::endl;
	}
	
	boost::this_thread::sleep_for(boost::chrono::seconds(10));

	return 0;
}