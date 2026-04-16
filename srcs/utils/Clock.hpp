#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <ctime>
#include <string>
#include <sys/time.h>

class Clock {
private:
	enum e_format { TIME, DATE, DATETIME };
	time_t _unixNow;
	struct tm *_now;
	long _microseconds;

	Clock &operator=(const Clock &other);
	Clock(const Clock &other);

public:
	// Constructors and destructors
	Clock();
	~Clock();

	// Methods
	std::string getFormatedTime(const int format);
	std::string nowDateTime();
	std::string nowDate();
	std::string nowTime();
};

#endif
