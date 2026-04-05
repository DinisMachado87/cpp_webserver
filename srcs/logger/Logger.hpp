#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Clock.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <climits>
#include <ctime>
#include <fstream>
#include <netinet/in.h>

class Logger {
	// STATIC SINGLETON
public:
	static Logger *_loggerPtr;
	static Logger *logger();

private:
	static const char *_labels[];

	// INTANCE BASED
public:
	typedef enum { NONE, ERROR, WARNING, DEBUG, LOG } e_logLevel;
	// Methods
	void logServer(const char *msg, const Server &server);
	void addHost(std::stringstream &stream, in_addr_t host);
	void info(const int level, const char *msg, std::stringstream &stream);
	void color(const int level, std::stringstream &stream);
	void print(const std::stringstream &stream);
	void log(const int level, const char *msg, const int socket = 0,
			 in_addr_t host = INT_MAX);
	void logTitle(const char *msg);

private:
	e_logLevel _level;
	Clock _clock;
	std::ofstream _logFile;
	// explicit disables
	Logger();
	Logger(const Logger &other);
	~Logger();
	Logger &operator=(const Logger &other);
};

#ifdef LOGGING
#define LOG(level, msg) Logger::logger()->log(level, msg, 0, INT_MAX)
#define LOGSOCK(level, msg, socket)                                            \
	Logger::logger()->log(level, msg, socket, INT_MAX)
#define LOGSOCKHOST(level, msg, socket, host)                                  \
	Logger::logger()->log(level, msg, socket, host)
#define LOG_TITLE(msg) Logger::logger()->logTitle(msg)
#define LOG_SERVER(msg, server) Logger::logger()->logServer(msg, server)
#else
#define LOG(level, msg) (void)0
#define LOGSOCK(level, msg, socket) (void)0
#define LOGSOCKHOST(level, msg, socket, host) (void)0
#define LOG_TITLE(msg) (void)0
#define LOG_SERVER(msg) (void)0
#endif
#endif
