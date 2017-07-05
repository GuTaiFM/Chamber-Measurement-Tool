#pragma once

#include <boost\asio.hpp>
#include <boost\asio\io_service.hpp>
#include <boost\asio\ip\tcp.hpp>

#include <boost\format.hpp>
#include <boost\scoped_ptr.hpp>
#include <boost\chrono.hpp>
#include <boost\thread\thread.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Declaration of constants to be used
enum AnalyserFormat {
	MLOG,
	PHAS,
	VSWR,
	SMIT
};

enum AnalyserParameter {
	S11,
	S12,
	S21,
	S22
};

enum AnalyserDataTransferFormat {
	REAL,
	REAL32
};

const std::map<AnalyserFormat, std::string> AnalyserFormatToStringMap{
	{MLOG, "MLOG"},
	{PHAS, "PHAS"},
	{VSWR, "VSWR"},
	{SMIT, "SMIT"}
};

const std::map<std::string, AnalyserFormat> StringToAnalyserFormatMap{
	{"MLOG", MLOG},
	{"PHAS", PHAS},
	{"VSWR", VSWR},
	{"SMIT", SMIT}
};

const std::map<AnalyserParameter, std::string> AnalyserParameterToStringMap{
	{S11, "S11"},
	{S12, "S12"},
	{S21, "S21"},
	{S22, "S22"}
};

const std::map<std::string, AnalyserParameter> StringToAnalyserParameterMap{
	{"S11", S11},
	{"S12", S12},
	{"S21", S21},
	{"S22", S22}
};

const std::map<AnalyserDataTransferFormat, std::string> AnalyserDataTransferFormatToStringMap{
	{REAL, "REAL"},
	{REAL32, "REAL32"}
};

const std::map<std::string, AnalyserDataTransferFormat> StringToAnalyserDataTransferFormatMap{
	{"REAL", REAL},
	{"REAL32", REAL32}
};

const std::map<AnalyserDataTransferFormat, int> AnalyserDataTransferFormatSize{
	{REAL, 8},
	{REAL32, 4}
};

/*
	Templated class for Analyser Object
*/
template<class T>
class AnalyserObj {
private:
	// Class specific constants
	const double MINFREQ = 100e3;
	const double MAXFREQ = 8.5e9;
	const double MINPOWERLVL = -55;
	const double MAXPOWERLVL = 10;
	const double MINIFBW = 2;
	const double MAXIFBW = 500e3;
	const int MINSAMPLEPOINTS = 2;
	const int MAXSAMPLEPOINTS = 1601;
	const int MINCHANNELS = 1;
	const int MAXCHANNELS = 16;
	const int MINTRACES = 1;
	const int MAXTRACES = 16;

	double m_startFreq;	// Start Frequency of the analyser
	double m_stopFreq; // Stop Frequency of the analyser
	double m_freqRange; // Frequency range

	double m_powerLvl; // Transmitted power level of the analyser
	double m_IFBW; // IFBW (Intermediate Frequency Bandwidth) of the analyser

	int m_samplePoints; // Number of points the analyser needs to sample
	int m_port; // The port which will be used by the socket to communicate with the analyser

	AnalyserFormat m_format; // Variable which stores the present analyser data formatting. Avoid having to query it every single time
	AnalyserParameter m_parameter; // Variable which stores the present analyser measurement parameter, i.e. S11, S21, etc...
	AnalyserDataTransferFormat m_dataTransferFormat; // Variable which defines the data format the analyser uses to transmit data to the computer

	std::string m_IP; // The IP address of the analyser

	std::vector<T> recvDataBuffer; // create a vector which will hold the received data

	boost::asio::io_service m_ioservice;
	boost::scoped_ptr<boost::asio::ip::tcp::socket> m_socket; // Boost ASIO socket object which will be used for socket programming
public:
	AnalyserObj(double startFreq = 100e3, double stopFreq = 8.5e9, double powerLvl = 0, double IFBW = 5e3, int samplePoints = 1601, AnalyserFormat format = MLOG, AnalyserParameter parameter = S21, AnalyserDataTransferFormat dtf = REAL32, std::string IP = "192.168.20.200", int port = 23);

	bool setStartFrequency(double startFreq = MINFREQ, int channel = 1);
	bool setStopFrequency(double stopFreq = MAXFREQ, int channel = 1);
	bool setFrequencyRange(double startFreq = MINFREQ, double stopFreq = MAXFREQ, int channel = 1);
	bool setPowerLvl(double powerLvl = 0, int port = 1);
	bool setIFBW(double IFBW = 5e3, int channel = 1);
	bool setSamplePoints(int samplePoints = MAXSAMPLEPOINTS, int channel = 1);
	bool setPort(int port = 22);
	bool setFormat(AnalyserFormat format = MLOG, int channel = 1);
	bool setParameter(AnalyserParameter parameter = S21, int channel = 1, int trace = 1);
	bool setIP(std::string ip = "192.168.20.200");
	bool setDataTransferFormat(AnalyserDataTransferFormat dtf = REAL32);

	double getStartFreq();
	double getStopFreq();
	double getPowerLvl();
	double getIFBW();
	int getSamplePoints();
	int getPort();
	AnalyserFormat getFormat();
	AnalyserParameter getParameter();
	AnalyserDataTransferFormat getDataTransferFormat();
	std::string getIP();

	std::vector<T> captureData(int channel = 1, int trace = 1);
	bool sendCommand(std::string command, int retryCount = 5);
	bool done();

	~AnalyserObj();
};

/*
	Because the AnalyserObj class is templated, it is necessary to put the function definitions of the class in the same header file as opposed to another cpp file.
	The class is templated to permit one to use various numerical data types with the functions without explicitly defining seperate functions to deal with floats or doubles.
*/

/*
	Constructor for the AnalyserObj object
*/
template<class T> AnalyserObj<T>::AnalyserObj(double startFreq, double stopFreq, double powerLvl, double IFBW, int samplePoints, AnalyserFormat format, AnalyserParameter parameter, AnalyserDataTransferFormat dtf, std::string IP, int port) {
	int retry_count = 5; // number of attempts to be made

	this->m_startFreq = startFreq;
	this->m_stopFreq = stopFreq;
	this->m_powerLvl = powerLvl;
	this->m_IFBW = IFBW;
	this->m_samplePoints = samplePoints;
	this->m_format = format;
	this->m_parameter = parameter;
	this->m_IP = IP;
	this->m_port = port;
	this->m_dataTransferFormat = dtf;

	try {
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(m_IP), m_port); // define the end point to which one will be connecting

		m_socket.reset(new boost::asio::ip::tcp::socket(m_ioservice, ep.protocol())); // initialise the socket object by resetting it.

		m_socket->connect(ep); // connect to the endpoint

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50)); // A small sleep delay to let things settle.

		sendCommand(":SYST:PRES"); // Send a command to preset the analyser to default values
		setFrequencyRange(m_startFreq, m_stopFreq); // Set the frequency range of the analyser
		setPowerLvl(m_powerLvl); // set the power level of the analyser
		setSamplePoints(m_samplePoints); // set the sample points of the analyser
		setFormat(m_format); // set the way data is formatted by the analyser
		setIFBW(m_IFBW); // Set the IFBW of the analyser
		setParameter(m_parameter); // Set the parameter the analyser must measure
		setDataTransferFormat(m_dataTransferFormat); // set the data format which the analyser must use to transmit data.

		while (!done()); // wait for the analyser to respond and say that the setup is complete

		std::cout << "The analyser has been successfully configured" << std::endl;
	}
	catch (boost::system::system_error &e) {
		std::cerr << "An error occured attempting to connect to the analyser" << std::endl;
		std::cerr << "Error code: " << e.code() << std::endl;
		std::cerr << "Error Message: " << e.what() << std::endl;

		throw e;
	}
}

/*
	Method to send the commands to the analyser.
*/
template<class T> bool AnalyserObj<T>::sendCommand(std::string command, int retryCount) {
	int commandLength = command.length(); // Store the length of the command

	try {
		// Here for debugging purposes
		std::cout << "Command: " << command << std::endl;

		// send the command and store the number of bytes sent
		size_t charsSent = m_socket->send(boost::asio::buffer(command));

		// Check whether the number of bytes corresponds to the command length. If not, something went wrong.
		// #TODO: add a retry loop which attempts sending the command a number of times, until all the bytes have been sent.
		if (charsSent == commandLength) {
			std::cout << "Command sent successfully" << std::endl;
			return true;
		}
		else {
			return false;
		}
	}
	catch (boost::system::system_error &e) {
		std::cerr << "An error occured whilst attempting to send the command to the analyser" << std::endl;
		std::cerr << "Error Code: " << e.code() << std::endl;
		std::cerr << "Error Message: " << e.what() << std::endl;

		throw e;
	}
}

/*
	Method for setting or changing the starting frequency of the analyser
	There are simple checks in place to ensure that the data is within the supported ranges of the Analyser
*/
template<class T> bool AnalyserObj<T>::setStartFrequency(double startFreq, int channel) {
	// Check whether the input values are within the range of the analyser and assign values accordingly
	if (startFreq < MINFREQ) {
		m_startFreq = MINFREQ;
	}
	else if (startFreq > MAXFREQ) {
		m_startFreq = MAXFREQ;
	}
	else {
		m_startFreq = startFreq;
	}

	// Create the command string to send to the analyser
	std::string command = boost::str(boost::format{ ":SENS%d:FREQ:STAR %f" } % channel % m_startFreq);

	// Return the outcome of the sendCommand function. A true will be returned 
	return sendCommand(command);
}

/*
	Method used to set the stop frequeny of the analyser
*/
template<class T> bool AnalyserObj<T>::setStopFrequency(double stopFreq, int channel) {
	// Check whether the input values are within the range of the analyser. Assign values accordingly.
	if (stopFreq < MINFREQ) {
		m_stopFreq = MINFREQ;
	}
	else if (stopFreq > MAXFREQ) {
		m_stopFreq = MAXFREQ;
	}
	else {
		m_stopFreq = stopFreq;
	}

	// Create command string to be sent to the analyser
	std::string command = boost::str(boost::format{ ":SENS%d:FREQ:STOP %f" } % channel % m_stopFreq);

	// return the outcome of the sendCommand method. 
	return sendCommand(command);
}

/*
	Method used to set the frequency range across which measurements will take place by passing the start and stop frequencies 
*/
template<class T> bool AnalyserObj<T>::setFrequencyRange(double startFreq, double stopFreq, int channel) {
	// Check whether the starting frequency is within the bounds of the analyser and assign values accordingly
	if (startFreq < MINFREQ) {
		m_startFreq = MINFREQ;
	}
	else if (startFreq > MAXFREQ) {
		m_startFreq = MAXFREQ;
	}
	else {
		m_startFreq = startFreq;
	}

	if (stopFreq < MINFREQ) {
		m_stopFreq = MINFREQ;
	}
	else if (stopFreq > MAXFREQ) {
		m_stopFreq = MAXFREQ;
	}
	else {
		m_stopFreq = stopFreq;
	}

	// Create a the command string which will be sent
	std::string command = boost::str(boost::format{ ":SENS%d:FREQ:STAR %f;:SENS%d:FREQ:STOP %f" } % channel % m_startFreq % channel % m_stopFreq);

	// return the outcome of the sendCommand method
	return sendCommand(command);
}

/*
	Method to set or change the transmitted power level of the analyser
*/
template<class T> bool AnalyserObj<T>::setPowerLvl(double powerLvl, int port) {
	if (powerLvl < MINPOWERLVL) {
		m_powerLvl = MINPOWERLVL;
	}
	else if (powerLvl > MAXPOWERLVL) {
		m_powerLvl = MAXPOWERLVL;
	}
	else {
		m_powerLvl = powerLvl;
	}


	std::string command = boost::str(boost::format{ ":SOUR%d:POW %f" } % port % m_powerLvl);

	return sendCommand(command);
}

/*
	Method to set of change the IFBW of the analyser
*/
template<class T> bool AnalyserObj<T>::setIFBW(double IFBW, int channel) {
	if (IFBW < MINIFBW) {
		m_IFBW = MINIFBW;
	}
	else if (IFBW > MAXIFBW) {
		m_IFBW = MAXIFBW;
	}
	else {
		m_IFBW = IFBW;
	}
	std::string command = boost::str(boost::format{ ":SENS%d:BWID %f" } % channel % m_IFBW);

	return sendCommand(command);
}

/*
	Method to set or change the number of sample points taken by the analyser
*/
template<class T> bool AnalyserObj<T>::setSamplePoints(int samplePoints, int channel) {
	if (samplePoints < MINSAMPLEPOINTS) {
		m_samplePoints = MINSAMPLEPOINTS;
	}
	else if (samplePoints > MAXSAMPLEPOINTS) {
		m_samplePoints = MAXSAMPLEPOINTS;
	}
	else {
		m_samplePoints = samplePoints;
	}

	std::string command = boost::str(boost::format{ ":SENS%d:SWE:POIN %d" } % channel % m_samplePoints);

	return sendCommand(command);
}

/*
	Method used to set or change he current port number
*/
template<class T> bool AnalyserObj<T>::setPort(int port) {
	try {
		// Define a new endpoint with the new port number
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string(m_IP), port);

		// Check whether a socket has already been created.
		if (m_socket->is_open()) {
			// If a socket exists, close it and reconnect to the new en point
			m_socket->close();
			m_socket->connect(ep);
		}
		else {
			// Otherwise, if a socket connection hasn't been made, then connect to the endpoint
			m_socket->connect(ep);
		}
	}
	catch (boost::system::system_error &e) {
		std::cerr << "An error has occurred whilst attempting to change the port of the Analyser Object" << std::endl;
		std::cerr << "Error Code: " << e.code() << std::endl;
		std::cerr << "Error Message: " << e.what() << std::endl;

		throw e;
	}
}

/*
	Method used to set the output format of the S-parameter data measured by the analyser
*/
template<class T> bool AnalyserObj<T>::setFormat(AnalyserFormat format, int channel) {
	if (format < 0) {
		m_format = MLOG;
	}
	else if (format > 3) {
		m_format = SMIT;
	}
	else {
		m_format = format;
	}

	std::string command = boost::str(boost::format( ":CALC%d:FORM %s" ) % channel % AnalyserFormatToStringMap.at(m_format));

	return sendCommand(command);
}

/* 
	Method to set or change the S-parameter which you wish to measure on the analyser
*/
template<class T> bool AnalyserObj<T>::setParameter(AnalyserParameter parameter, int channel, int trace) {
	if (parameter < 0) {
		m_parameter = S11;
	}
	else if (parameter > 3) {
		m_parameter = S22;
	}
	else {
		m_parameter = parameter;
	}

	std::string command = boost::str(boost::format{ ":CALC%d:PAR%d:DEF %s" } % channel % trace % AnalyserParameterToStringMap.at(m_parameter));

	return sendCommand(command);
}

/*
 Method to set or change the IP address which points to the analyser
*/
template<class T> bool AnalyserObj<T>::setIP(std::string ip) {
	try {
		// Create and object which holds the information for the endpoint which we wish to connect to
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(ip), m_port);

		// Check whether a socket has been opened already
		if (m_socket->is_open()) {
			// If a socket is already open, close the socket and reconnect using the new endpoint data
			m_socket->close();
			m_socket->connect(ep);
		}
		else {
			// Otherwise, just connect to the enpoint
			m_socket->connect(ep);
		}
	}
	catch (boost::system::system_error &e) {
		std::cerr << "There was an error attempting to close the socket" << std::endl;
		std::cerr << "Error Code: " << e.code() << std::endl;
		std::cerr << "Error Message: " << e.what() << std::endl;

		throw e;
	}
}

/*
	Method to set or change the Data Transfer Format of the analyser. The Data Transfer Format dictates the data format used by the analyser
	to send data to the computer. It is important to know what the data format is, as this allows one to easily calculate the amount of data to expect
*/
template<class T> bool AnalyserObj<T>::setDataTransferFormat(AnalyserDataTransferFormat dtf) {
	if (dtf < 0) {
		m_dataTransferFormat = REAL;
	}
	else if (dtf > 1) {
		m_dataTransferFormat = REAL32;
	}
	else {
		m_dataTransferFormat = dtf;
	}


	std::string command = boost::str(boost::format( ":FORM:DATA %s" ) % AnalyserDataTransferFormatToStringMap.at(m_dataTransferFormat));

	return sendCommand(command);
}

/*
	Method used to request data from the analyser and return a vector containing the received data.
*/
template<class T> std::vector<T> AnalyserObj<T>::captureData(int channel, int trace) {
	std::string header;
	int samplesReceived = 0;
	int totalSamplesReceived = 0;

	// Tell the analyser to wait for an external trigger to request data
	// If the command fails, just return an empty vector
	if (!sendCommand(":TRIG:SOUR EXT")) {
		return{};
	}

	// Send a command to the analyser requesting that it captures a single sweep and then sends the data to the computer
	if (!sendCommand(":TRIG:SING")) {
		return{};
	}

	// Wait for the analyser to finish
	while (!done());

	// Loop until all the sampled data has been received. The amount of sampled data which we expect is equal to the number of sample points times 2.
	// The reason for this is because all the data return is sent as complex data, even if there is no complex component. 
	while (totalSamplesReceived < (2 * m_samplePoints)) {
		// First it is necessary to process the header of the received data.
		// The header tells us how much information we've received from the analyser, thus we can use this information to determine whether we have received all the data or not. 
		// #TODO: If the correct amount of data is not received, retry.
		m_socket->receive(boost::asio::buffer(header, 8));
		std::stringstream(header) >> samplesReceived;

		samplesReceived /= dataSize[m_dataTransferFormat];

		m_socket->receive(boost::asio::buffer(dataBuffer, samplesReceived));
	}

	return dataBuffer;
}

/*
	Method for checking whether the analyser has finished processing the last command sent to it
*/
template<class T> bool AnalyserObj<T>::done() {
	long int response = 0;
	
	// The *OPC? command queries the analyser to check whether the last command has been processed. The received response is +1
	this->sendCommand("*OPC?");
	m_socket->receive(boost::asio::buffer(&response, 1));
	return (response == 1);
}

/*
	Destructor method for the analyser object. This method is called when the object goes out of scope. This method is needed to close the socket and prevent unexpected behaviour
*/
template<class T> AnalyserObj<T>::~AnalyserObj() {
	if (m_socket->is_open())
		m_socket->close();
}

/*
	Getter methods
*/

template<class T> double AnalyserObj<T>::getStartFreq() {
	return m_startFreq;
}

template<class T> double AnalyserObj<T>::getStopFreq() {
	return m_stopFreq;
}

template<class T> double AnalyserObj<T>::getPowerLvl() {
	return m_powerLvl;
}

template<class T> double AnalyserObj<T>::getIFBW() {
	return m_IFBW;
}

template<class T> int AnalyserObj<T>::getSamplePoints() {
	return m_samplePoints;
}

template<class T> int AnalyserObj<T>::getPort() {
	return m_port;
}

template<class T> AnalyserFormat AnalyserObj<T>::getFormat() {
	return m_format;
}

template<class T> AnalyserParameter AnalyserObj<T>::getParameter() {
	return m_parameter;
}

template<class T> std::string AnalyserObj<T>::getIP() {
	return m_IP;
}

template<class T> AnalyserDataTransferFormat AnalyserObj<T>::getDataTransferFormat() {
	return m_dataTransferFormat;
}