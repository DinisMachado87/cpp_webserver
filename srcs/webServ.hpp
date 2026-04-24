#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <csignal>
#define VERBOSE 1
// Unix return code
#define OK 0
#define ERR -1
// Multical returns
#define ONGOING false
#define DONE true
// Allow
#define ALLOW true
#define FORBID false
// EPOLL Macros
#define MAX_EVENTS 1024
#define RESPONSES_CUE_SIZE 10
#define TIMEOUT 1000
#define RECV_SIZE 1000
#define CHUNK_SIZE 1000
// logger
#define LOGLEVEL CONTENT
#define LOGGING true
#define LOGTOCLI true
#define LOGTOFILE true
#define NONUM -2

// HTTP Parser Limits
#define MAX_HEADER_SIZE 8192
#define MAX_CONTENT_LENGTH 1024

// #define DEBUG_PATH true

#ifdef DEBUG_PATH
#define DEBUG(x) x
#else
#define DEBUG(x) ((void)0)
#endif

// Type Macros
typedef unsigned int uint;
typedef unsigned char uchar;

extern volatile sig_atomic_t g_shutdown;

extern const char *g_methods[];

#endif
