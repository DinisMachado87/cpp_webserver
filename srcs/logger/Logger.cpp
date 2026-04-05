#include "Logger.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <climits>
#include <cstddef>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <istream>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#define COLOR_DEBUG "\033[36m" // Cyan
#define COLOR_INFO "\033[32m"  // Green
#define COLOR_WARN "\033[33m"  // Yellow
#define COLOR_ERROR "\033[31m" // Red
#define COLOR_PURPLE "\033[35m"
#define COLOR_RESET "\033[0m"
#define SEPARATOR "\t|"

using std::cout;
using std::endl;
using std::runtime_error;
using std::string;
using std::stringstream;

Logger *Logger::_loggerPtr = NULL;
const char *Logger::_labels[] = {"[NONE]", "[ERROR]", "[WARNING]", "[DEBUG]",
								 "[LOG]"};
// Public constructors and destructors
Logger::Logger() :
	_level(LOGLEVEL) {}

Logger::~Logger() { _logFile.close(); }

// Public Methods
Logger *Logger::logger() {
	if (!_loggerPtr) {
		_loggerPtr = new Logger;
		if (!_loggerPtr)
			throw runtime_error("Error creating logger instance");

		_loggerPtr->_logFile.open("serverLog.txt", std::iostream::app);
		if (!_loggerPtr->_logFile.is_open())
			throw runtime_error("Error opening logfile");
	}
	return _loggerPtr;
}

void Logger::print(const stringstream &stream) {
	if (LOGTOCLI)
		cout << stream.str() << std::flush;
	if (LOGTOFILE)
		_logFile << stream.str() << std::flush;
}

void Logger::color(const int level, stringstream &stream) {
	switch (level) {
	case ERROR:
		stream << COLOR_ERROR;
		break;
	case WARNING:
		stream << COLOR_WARN;
	default:
		return;
	}
}

void Logger::info(const int level, const char *msg, stringstream &stream) {
	stream << _labels[level] << msg << COLOR_RESET;
}

void Logger::addHost(stringstream &stream, in_addr_t host) {
	stream << " | Host: ";
	uchar *octet = (uchar *)&host;
	stream << (int)octet[0] << '.' << (int)octet[1] << '.' << (int)octet[2]
		   << '.' << (int)octet[3];
}

void Logger::log(const int level, const char *msg, const int socket,
				 in_addr_t host) {
	if (level < _level)
		return;

	stringstream stream;
	stream << _clock.now() << " | ";
	info(level, msg, stream);
	if (socket)
		stream << " | Socket: " << socket;
	if (host != INT_MAX)
		addHost(stream, host);
	stream << '\n';
	print(stream);
}

void Logger::logTitle(const char *msg) {
	stringstream stream;
	stream << COLOR_PURPLE << "=====" << msg << "=====" << COLOR_RESET << endl;
	print(stream);
}

void Logger::logServer(const char *msg, const Server &server) {
	stringstream stream;
	stream << msg << '\n';
	server.getServerStr(stream);
	print(stream);
}
