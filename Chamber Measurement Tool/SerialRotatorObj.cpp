#include "SerialRotatorObj.h"
#include "SerialRotatorException.h"
#include <boost\format.hpp>
#include <boost\optional.hpp>
#include <boost\timer.hpp>
#include <boost\chrono.hpp>
#include <boost\bind.hpp>
#include <iostream>
#include <cmath>
#include <array>
#include <algorithm>

/*
	Templated signum function used for obtaining the sign of a value
	This accepts any numeric value such as char, int, float, double, etc and determines the sign of the value and returns a positive or negative integer
	which dictates the sign
*/
template<typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

/*
	Constructor of SerialRotatorObj constructor
	This initialises the SerialRotatorObj with the supplied initial parameters.
	Some of the input parameters of function have default values assigned to them. Refer back to SerialObj.h for the class declaration and the defualt parameters
*/
SerialRotatorObj::SerialRotatorObj(unsigned char speed, unsigned char accel, double stepAngle, unsigned char COMPort, int baudrate)  : RotatorObj(speed, accel, stepAngle){
	this->m_COMPort = COMPort;
	this->baudrate = baudrate;
	this->m_currentPosition = 0.0;

	m_serialConn.reset(new boost::asio::serial_port(m_ios)); // This initialises the Serial Object with the ios object

	try {
		boost::system::error_code ec;

		m_serialConn->open(boost::str(boost::format("COM%d") % this->m_COMPort)); // open serial port with parameter COM[COMPort]. Format function is used to convert COMPort number to string

		m_serialConn->set_option(boost::asio::serial_port_base::baud_rate(this->baudrate)); // Set the baudrate required for communication
		m_serialConn->set_option(boost::asio::serial_port_base::character_size(8)); // set the character length of serial communications. This is 8 bits by default as opposed to 9 bits
		m_serialConn->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one)); // set the number of stop bits. This is set to 1 by default
		m_serialConn->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none)); // set the number of parity bits. This is set to none by default

		std::array<unsigned char, 5> command = { 1, this->m_speed, this->m_accel, 3, 4 }; // create integer array which holds the commands which will be sent to the rotator for initialisation

		boost::asio::write(*m_serialConn, boost::asio::buffer(command, 5)); // Write the initialisation command to the rotator

		std::array<unsigned char, 5> reply; // create a character array which waits for the response from the rotator

		boost::asio::read(*m_serialConn, boost::asio::buffer(reply, 5), boost::asio::transfer_all()); // transfer the data in the serial buffer to the reply array

		/*
			If the rotator does not return the sent command to the user, then an exception needs to be thrown
		*/
		if (!std::equal(command.begin(), command.end(), reply.begin())) {
			throw SerialRotatorException("Unable to change rotator speed. Did not receive the correct reply from the rotator");
		}
	}
	catch (boost::system::system_error &e) {
		std::cerr << "There was an error attempting to connect to the rotator" << std::endl;

		throw(e);
	}
}

/*
	THis function is used to send a command to the rotator to rotate the rotator by some angle.
*/
void SerialRotatorObj::rotateBy(RotatorDirection direction, double angle, bool wait) {
	// Check whether the input angle is greater than the minimum resolution of the rotator
	if (std::abs(angle) > 0.01) {

		this->m_currentPosition += direction * angle; // update the current position of the rotator for client tracking purposes

		int rotationSteps = static_cast<int>(std::round(angle * 2e5 / 360.0)); // calculate the number of rotation steps needed to be taken by the rotator controller
		std::array<unsigned char, 3> cAngle;

		/*
			Calculation of the values which form part of the command which will be sent to the rotator
		*/
		cAngle[0] = static_cast<unsigned char>(std::floor(rotationSteps / std::powf(2.0, 16)));
		cAngle[1] = static_cast<unsigned char>(std::floor((rotationSteps - cAngle[0] * std::powf(2.0, 16)) / std::powf(2.0, 8)));
		cAngle[2] = static_cast<unsigned char>(std::round(std::fmod(rotationSteps, std::powf(2.0, 8))));

		unsigned char moveCommand[5];

		// The values are assigned to the move command which will be sent.

		moveCommand[1] = direction;
		moveCommand[2] = cAngle[0];
		moveCommand[3] = cAngle[1];
		moveCommand[4] = cAngle[2];

		// The first value of the move command changes according to whether one waits for the rotator to finish or whether one is going to send commands before the analyser finishes moving.

		if (wait) {
			moveCommand[0] = 3;
		}
		else {
			moveCommand[0] = 2;
		}

		// The try block where the data will be transmitted to the rotator
		try {
			unsigned char reply = 0;
			boost::system::error_code ec;
			
			boost::asio::write(*m_serialConn, boost::asio::buffer(moveCommand, 5)); // send command to rotator

			/*
				Wait for the rotator to reply
			*/
			while ((reply != 2) || (reply != 3)) {
				boost::asio::read(*m_serialConn, boost::asio::buffer(&reply, 1));
			}
		}
		catch (boost::system::system_error &e) {
			std::cerr << "There was a problem attempting to rotate to the requested position" << std::endl;
			throw(e);
		}
	}
}


/*
	Rotate to a specific angle
*/
void SerialRotatorObj::rotateTo(double position, bool wait) {
	double rotateAngle = position - this->m_currentPosition;

	rotateBy(static_cast<RotatorDirection>(sgn(rotateAngle)), rotateAngle, wait);
}

/*
	Overridden function for setSpeed to set the internal variable and send a command to change the rotation speed
*/
void SerialRotatorObj::setSpeed(unsigned char speed) {
	this->setSpeed(speed); // set internal value of internal variable

	std::array<unsigned char, 5> command = { 1, this->m_speed, this->m_accel, 3, 4 };  // create the command to be sent to the rotator

	// try to send the command
	try {
		boost::asio::write(*m_serialConn, boost::asio::buffer(command, 5));

		std::array<unsigned char, 5> reply;

		boost::asio::read(*m_serialConn, boost::asio::buffer(reply, 5));

		if (!std::equal(command.begin(), command.end(), reply.begin())) {
			throw SerialRotatorException("Unable to change rotator speed. Did not receive the correct reply from the rotator");
		}
	}
	catch (boost::system::system_error &e) {
		std::cerr << "There was an issue whilst attempting to set the rotator speed" << std::endl;
		throw(e);
	}
}

/*
	Overridden function for setAccel to set the internal variable and send the command to change the acceleration
*/
void SerialRotatorObj::setAccel(unsigned char accel) {
	this->setAccel(accel);

	std::array<unsigned char, 5> command = { 1, this->m_speed, this->m_accel, 3, 4 };

	try {
		boost::asio::write(*m_serialConn, boost::asio::buffer(command, 5));

		std::array<unsigned char, 5> reply;

		boost::asio::read(*m_serialConn, boost::asio::buffer(reply, 5));

		if (!std::equal(command.begin(), command.end(), reply.begin())) {
			throw SerialRotatorException("Unable to change rotator speed. Did not receive the correct reply from the rotator");
		}
	}
	catch (boost::system::system_error &e) {
		std::cerr << "There was an issues whilst attempting to set the rotator acceleration" << std::endl;
		throw(e);
	}
}

void SerialRotatorObj::setStepAngle(double stepAngle) {
	this->m_stepAngle = stepAngle;
}

void SerialRotatorObj::setCurrentPosition(double currentPosition) {
	this->m_currentPosition = currentPosition;
}

unsigned char SerialRotatorObj::getSpeed() {
	return m_speed;
}

unsigned char SerialRotatorObj::getAccel() {
	return m_accel;
}

double SerialRotatorObj::getStepAngle() {
	return m_stepAngle;
}

double SerialRotatorObj::getCurrentPosition() {
	return m_currentPosition;
}

// Destructor function for SerialRotatorObj. This function will close the serial comm object once this object is destroyed
SerialRotatorObj::~SerialRotatorObj() {
	if (m_serialConn->is_open()) {
		m_serialConn->close();
	}
}