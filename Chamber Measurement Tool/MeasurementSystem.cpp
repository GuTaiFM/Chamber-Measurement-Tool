#include "MeasurementSystem.h"

MeasurementSystem::MeasurementSystem(AnalyserObj<double> *analyser = nullptr, SerialRotatorObj *rotator = nullptr){
	
	if (!analyser) {
		std::cout << "The analyser object is not pointing to anything. No analyser object was assigned to the MeasurementSystem object" << std::endl;
		std::cout << "The rotator object is not pointing to anything. No rotator object was assigned to the MeasurementSystem object" << std::endl;
	}
	else {
		this->analyser.reset(analyser);
		return;
	}
	
	if (!rotator) {
		std::cout << "The rotator object is not pointing to anything. No rotator object was assigned to the MeasurementSystem object" << std::endl;
	}
	else {
		this->rotator.reset(rotator);
		return;
	}

}