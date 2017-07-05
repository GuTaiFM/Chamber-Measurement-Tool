#pragma once
#include "AnalyserObj.h"
#include "SerialRotatorObj.h"

class MeasurementSystem {
private:
	boost::scoped_ptr<AnalyserObj<double>> analyser;
	boost::scoped_ptr<SerialRotatorObj> rotator;

public:
	MeasurementSystem(AnalyserObj<double> *analyser, SerialRotatorObj *rotator);

};