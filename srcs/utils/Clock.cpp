#include "Clock.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/time.h>

using std::strftime;
using std::string;
using std::stringstream;

// Public constructors and destructors
Clock::Clock() :
	_unixNow(0),
	_now(NULL),
	_microseconds(0) {}
Clock::~Clock() {}

string Clock::getFormatedTime(const int format) {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	_unixNow = tv.tv_sec;
	_microseconds = tv.tv_usec;
	_now = localtime(&_unixNow);

	char buffer[80];
	stringstream ss;

	switch (format) {
	case DATE:
		strftime(buffer, sizeof(buffer), "%Y-%m-%d", _now);
		ss << buffer;
		break;
	case TIME:
		strftime(buffer, sizeof(buffer), "%H:%M:%S", _now);
		ss << buffer << "." << std::setfill('0') << std::setw(3)
		   << (_microseconds / 1000);
		break;
	case DATETIME:
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", _now);
		ss << buffer << "." << std::setfill('0') << std::setw(3)
		   << (_microseconds / 1000);
		break;
	}
	return ss.str();
}

// Public Methods
string Clock::nowTime() { return getFormatedTime(TIME); }
string Clock::nowDate() { return getFormatedTime(DATE); }
string Clock::nowDateTime() { return getFormatedTime(DATETIME); }
