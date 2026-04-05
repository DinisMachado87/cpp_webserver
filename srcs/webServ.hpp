#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define VERBOSE 1
// Unix return code
#define OK 0
#define ERR 1
// EPOLL Macros
#define MAX_EVENTS 1024
#define TIMEOUT 1000
#define CHUNK_SIZE 1000
// logger
#define LOGLEVEL LOG
#define LOGGING true
#define LOGTOCLI true
#define LOGTOFILE true

typedef unsigned int uint;
typedef unsigned char uchar;

#endif
