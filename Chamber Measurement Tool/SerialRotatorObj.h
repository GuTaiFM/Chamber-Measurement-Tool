#pragma once
#include "RotatorObj.h"
#include <boost\scoped_ptr.hpp>
#include <boost\asio.hpp>
#include <boost\asio\serial_port.hpp>

/*
	Enumeration object used to determine the rotation direction of the rotator
	using words, as opposed to number. This helps to establish a consistent convention
	throughout the code
*/
enum RotatorDirection {
	ANTICLOCKWISE = -1, 
	CLOCKWISE = 1
};

/*
	Create serialRotatorObj which is derived from a RotatorObj, but the connection is specialised for Serial communications
*/
class SerialRotatorObj : public RotatorObj {
private:
	int m_COMPort; // The serial COM port over which communication which will take place between the computer and the rotator
	int baudrate; // The agreed upon data rate between the computer and the serial rotator
	 
	boost::asio::io_service m_ios; // This is the worker class for Boost's IO communications library. This is needed to perform the necessary communication functions
	boost::scoped_ptr<boost::asio::serial_port> m_serialConn; // This creates a serial communications object

public:
	SerialRotatorObj(unsigned char speed = 1, unsigned char accel = 255, double stepAngle = 5, unsigned char COMPort = 4, int baudrate = 9600);
	void rotateBy(RotatorDirection direction, double angle, bool wait = 1);
	void rotateTo(double position, bool wait = 1);
	void setSpeed(unsigned char speed = 255);
	void setAccel(unsigned char accel = 1);
	void setStepAngle(double stepAngle = 5.0);
	void setCurrentPosition(double currentPosition);

	unsigned char getSpeed();
	unsigned char getAccel();
	double getStepAngle();
	double getCurrentPosition();

	~SerialRotatorObj();
};