#include <boost\asio\io_service.hpp>
#include <boost\asio\ip\tcp.hpp>
#include <boost\format.hpp>
#include <boost\scoped_ptr.hpp>
#include <string>
#include <vector>

#pragma once

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

std::string AnalyserFormatToString(AnalyserFormat format) {
	switch (format) {
	case MLOG:
		return "MLOG";
	case PHAS:
		return "PHAS";
	case VSWR:
		return "VSWR";
	case SMIT:
		return "SMIT";
	default:
		return "Invalid";
	}
}

std::string AnalyserParametersToString(AnalyserParameter parameter) {
	switch (parameter) {
	case S11:
		return "S11";
	case S12:
		return "S12";
	case S21:
		return "S21";
	case S22:
		return "S22";
	default:
		return "Invalid";
	}
}

std::string AnalyserDataTransferFormatToString(AnalyserDataTransferFormat dtf) {
	switch (dtf) {
	case REAL:
		return "REAL";
	case REAL32:
		return "REAL32";
	default:
		return "Invalid";
	}
}

int getDataSize(AnalyserDataTransferFormat format) {
	switch (format) {
	case REAL:
		return 8;
	case REAL32:
		return 4;
	default:
		return 1;
	}
}

template<class T>
class AnalyserObj {
private:
	double m_startFreq;
	double m_stopFreq;
	double m_range;

	double m_powerLvl;
	double m_IFBW;

	int m_samplePoints;
	int m_port;

	AnalyserFormat m_format;
	AnalyserParameter m_parameter;
	AnalyserDataTransferFormat m_dataTransferFormat;

	std::string m_IP;

	boost::asio::io_service m_ioservice;
	boost::scoped_ptr<boost::asio::ip::tcp::socket> m_socket;
public:
	AnalyserObj(double startFreq, double stopFreq, double powerLvl, double IFBW, int samplePoints, AnalyserFormat format, AnalyserParameter parameter, AnalyserDataTransferFormat dtf, std::string IP, int port);

	bool setStartFrequency(double startFreq, int channel = 1);
	bool setStopFrequency(double stopFreq, int channel = 1);

	bool setFrequencyRange(double startFreq, double stopFreq, int channel = 1);
	bool setPowerLvl(double powerLvl, int port = 1);
	bool setIFBW(double IFBW, int channel = 1);
	bool setSamplePoints(int samplePoints, int channel = 1);
	bool setPort(int port);
	bool setFormat(AnalyserFormat format, int channel = 1);
	bool setParameter(AnalyserParameter parameter, int channel = 1, int trace = 1);
	bool setIP(std::string ip);
	bool setDataTransferFormat(AnalyserDataTransferFormat dtf);

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