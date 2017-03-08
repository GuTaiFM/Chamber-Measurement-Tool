#include "AnalyserObj.h"
#include <boost\chrono\chrono.hpp>
#include <boost\asio\write.hpp>
#include <boost\asio\buffer.hpp>
#include <boost\thread\thread.hpp>
#include <iostream>

template<class T> AnalyserObj<T>::AnalyserObj(double startFreq, double stopFreq, double powerLvl, double IFBW, int samplePoints, AnalyserFormat format, AnalyserParameter parameter, AnalyserDataTransferFormat dtf, std::string IP, int port) {
	int retry_count = 5;
	
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
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(m_IP), m_port);

		m_socket.reset(new boost::asio::ip::tcp::socket(m_ioservice, ep.protocol()));

		m_socket->connect(ep);

		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

		sendCommand(":SYST:PRES");
		setFrequencyRange(startFreq, stopFreq);
		setPowerLvl(powerLvl);
		setSamplePoints(samplePoints);
		setFormat(format);
		setIFBW(IFBW);
		setParameter(parameter);
		setDataTransferFormat(dtf);

		while (!done());

		std::cout << "The analyser has been successfully configured" << std::endl;
	}
	catch (boost::system::system_error &e) {
		std::cout << "An error occured attempting to connect to the analyser" << std::endl;
		std::cout << "Error code: " << e.code() << std::endl;
		std::cout << "Error Message: " << e.what() << std::endl;

		return;
	}

}

template<class T> bool AnalyserObj<T>::sendCommand(std::string command, int retryCount) {
	int commandLength = command.length();

	try {
		std::cout << "Command: " << command << std::endl;

		size_t charsSent = m_socket->send(boost::asio::buffer(command));

		if (charsSent == commandLength) {
			std::cout << "Command sent successfully" << std::endl;
			return true;
		}
		else {
			return false;
		}
	}
	catch (boost::system::system_error &e) {
		std::cout << "An error occured whilst attempting to send the command to the analyser" << std::endl;
		std::cout << "Error Code: " << e.code() << std::endl;
		std::cout << "Error Message: " << e.what() << std::endl;
		
		return false;
	}
}

template<class T> bool AnalyserObj<T>::setStartFrequency(double startFreq, int channel) {
	std::string command = boost::str(boost::format{ ":SENS%d:FREQ:STAR %f" } % channel % startFreq);
	
	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_startFreq = startFreq;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setStopFrequency(double stopFreq, int channel) {
	std::string command = boost::str(boost::format{ ":SENS%d:FREQ:STOP %f" } % channel % stopFreq);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_stopFreq = stopFreq;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setFrequencyRange(double startFreq, double stopFreq, int channel) {

	std::string command = boost::str(boost::format{ ":SENS%d:FREQ:STAR %f;:SENS%d:FREQ:STOP %f" } % channel % startFreq % channel % stopFreq);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_startFreq = startFreq;
		m_stopFreq = stopFreq;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setPowerLvl(double powerLvl, int port) {
	std::string command = boost::str(boost::format{ ":SOUR%d:POW %f" } % port % powerLvl);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_powerLvl = powerLvl;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setIFBW(double IFBW, int channel) {
	std::string command = boost::str(boost::format{ ":SENS%d:BWID %f" } % channel % IFBW);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_IFBW = IFBW;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setSamplePoints(int samplePoints, int channel) {
	std::string command = boost::str(boost::format{ ":SENS%d:SWE:POIN %d" } % channel % samplePoints);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_samplePoints = samplePoints;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setPort(int port) {
	if (m_socket->is_open()) {
		try {
			m_socket->close();
			boost::this_thread::sleep_for(boost::chrono::seconds(1));
			boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string(m_IP), m_port);
			m_socket->connect(ep);
		}
		catch (boost::system::system_error &e) {
			std::cout << "An error has occurred whilst attempting to close the socket" << std::endl;
			std::cout << "Error Code: " << e.code() << std::endl;
			std::cout << "Error Message: " << e.what() << std::endl;

			return false;
		}
		
	}

	try {
		if (m_socket->is_open()) {
			m_socket->close();

			m_socket.reset(new boost::asio::ip::tcp::socket())
		}

		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}
	catch (boost::system::system_error &e) {
		std::cout << "An error occured whilst attempting to open the new socket" << std::endl;
		std::cout << "Error Code: " << e.code() << std::endl;
		std::cout << "Error Message: " << e.what() << std::endl;

		return false;
	}

	m_port = port;

	return true;
}

template<class T> bool AnalyserObj<T>::setFormat(AnalyserFormat format, int channel) {
	std::string command = boost::str(boost::format{ ":CALC%d:FORM %s" } % channel % format);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_format = format;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setParameter(AnalyserParameter parameter, int channel, int trace) {
	std::string command = boost::str(boost::format{ ":CALC%d:PAR%d:DEF %s" } % channel % trace % parameter);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_parameter = parameter;
		return true;
	}
}

template<class T> bool AnalyserObj<T>::setIP(std::string ip) {
	if (m_socket->is_open()) {
		try {
			m_socket->close();
			boost::this_thread::sleep_for(boost::chrono::seconds(1));
			boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(m_IP), m_port);
			m_socket->connect(ep);
		}
		catch (boost::system::system_error &e) {
			std::cout << "There was an error attempting to close the socket" << std::endl;
			std::cout << "Error Code: " << e.code() << std::endl;
			std::cout << "Error Message: " << e.what() << std::endl;

			return false;
		}
	}

	try {

		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}
	catch (boost::system::system_error &e) {
		std::cout << "There was an error attempting to open the socket" << std::endl;
		std::cout << "Error Code: " << e.code() << std::endl;
		std::cout << "Error Message: " << e.what() << std::endl;

		return false;
	}

	m_IP = ip;

	return true;
}

template<class T> bool AnalyserObj<T>::setDataTransferFormat(AnalyserDataTransferFormat dtf) {
	std::string command = boost::str(boost::format{ ":FORM:DATA %s" } % dtf);

	if (!sendCommand(command)) {
		return false;
	}
	else {
		m_dataTransferFormat = dtf;
		return true;
	}
}

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

template<class T> std::vector<T> AnalyserObj<T>::captureData(int channel, int trace) {
	std::string header;

	int samplesReceived = 0;

	if (!sendCommand(":TRIG:SOUR EXT")) {
		return {};
	}

	if (!sendCommand(":TRIG:SING")) {
		return{};
	}

	while (!done());

	int totalSamplesReceived = 0;

	while (totalSamplesReceived < (2 * m_samplePoints)) {
		m_socket->receive(boost::asio::buffer(header, 8));
		std::stringstream(header) >> samplesReceived;
		
		samplesReceived /= dataSize[m_dataTransferFormat];

		m_socket->receive(boost::asio::buffer(dataBuffer, samplesReceived));
	}

	return dataBuffer;
}

template<class T> bool AnalyserObj<T>::done() {
	this->sendCommand("*OPC?");

	std::string response;

	m_socket->receive(boost::asio::buffer(response, 4));

	if (response.compare("+1\r\n")) {
		return true;
	}
	else {
		return false;
	}
}

template<class T> AnalyserObj<T>::~AnalyserObj() {
	if(m_socket->is_open())
		m_socket->close();
}