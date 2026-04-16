#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Clock.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <climits>
#include <cstddef>
#include <ctime>
#include <fstream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>

class Logger {
	// STATIC SINGLETON
public:
	static Logger *_loggerPtr;
	static Logger *logger();
	static void deleteLogger();

private:
	static const char *_labels[];

	// INTANCE BASED
public:
	typedef enum {
		NONE,
		ERROR,
		WARNING,
		DEBUG,
		TITLE,
		LOG,
		CONTENT
	} e_logLevel;
	// Methods
	const char *color(const int level);
	std::string traced(const char *msg, const char *file, const int line,
					   const char *func);
	void print(const int level, std::stringstream &stream);
	void logError(const char *label, std::runtime_error errorMsg,
				  const int socket = 0);
	void info(const int level, const std::runtime_error &msg,
			  std::stringstream &stream);
	void logServer(const char *msg, const Server &server);
	void addHost(std::stringstream &stream, in_addr_t host);
	void info(const int level, const char *msg, std::stringstream &stream);
	void log(const int level, const char *msg, size_t len, const int num,
			 const int socket = 0, in_addr_t host = INT_MAX);
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
#define LOG(level, msg) Logger::logger()->log(level, msg, 0, NONUM, 0, INT_MAX)
#define LOGTRUNC(level, msg, len)                                              \
	Logger::logger()->log(level, msg, len, NONUM, 0, INT_MAX)
#define LOGNUM(level, msg, num)                                                \
	Logger::logger()->log(level, msg, 0, num, 0, INT_MAX)
#define LOGTRUNCNUM(level, msg, len, num)                                      \
	Logger::logger()->log(level, msg, len, num, 0, INT_MAX)
#define LOGSOCK(level, msg, socket)                                            \
	Logger::logger()->log(level, msg, 0, NONUM, socket, INT_MAX)
#define LOGSOCKTRUNC(level, msg, len, socket)                                  \
	Logger::logger()->log(level, msg, len, NONUM, socket, INT_MAX)
#define LOGSOCKNUM(level, msg, num, socket)                                    \
	Logger::logger()->log(level, msg, 0, num, socket, INT_MAX)
#define LOGSOCKHOST(level, msg, socket, host)                                  \
	Logger::logger()->log(level, msg, 0, NONUM, socket, host)
#define LOG_TITLE(msg) Logger::logger()->logTitle(msg)
#define LOG_SERVER(msg, server) Logger::logger()->logServer(msg, server)
#define LOG_ERROR(errMsg) Logger::logger()->logError(NULL, errMsg)
#define LOG_ERROR_SOCK(errMsg, socket)                                         \
	Logger::logger()->logError(NULL, errMsg, socket)
#define LOG_ERROR_LABELED(label, errMsg)                                       \
	Logger::logger()->logError(label, errMsg)
#define LOG_ERROR_SOCK_LABELED(label, errMsg, socket)                          \
	Logger::logger()->logError(label, errMsg, socket)
#define TRACED(msg)                                                            \
	Logger::logger()->traced(msg, __FILE__, __LINE__, __FUNCTION__)
#else
#define LOG(level, msg) (void)0
#define LOGTRUNC(level, msg, len) (void)0
#define LOGNUM(level, msg, num) (void)0
#define LOGTRUNCNUM(level, msg, len, num) (void)0
#define LOGSOCK(level, msg, socket) (void)0
#define LOGSOCKTRUNC(level, msg, len, socket) (void)0
#define LOGSOCKNUM(level, msg, socket, num) (void)0
#define LOGSOCKHOST(level, msg, socket, host) (void)0
#define LOG_TITLE(msg) (void)0
#define LOG_SERVER(msg) (void)0
#define LOG_ERROR(errMsg) (void)0
#define LOG_ERROR_SOCK(errMsg) (void)0
#define LOG_ERROR_LABELED(label, errMsg) (void)0
#define LOG_ERROR_SOCK_LABELED(label, errMsg) (void)0
#define TRACED(str, __FILE__, __LINE__, __FUNCTION__) (void)0
#endif
#endif
