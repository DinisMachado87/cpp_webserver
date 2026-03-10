#include "Connection.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>

using std::runtime_error;
using std::string;

// Public constructors and destructors
Connection::Connection(int fd, const Server& server, struct sockaddr_in serverAddr):
	ASocket(fd, server, serverAddr) {}

Connection::~Connection() {}

// Public Methods
Connection*	Connection::handleIn() { return NULL; };
void		Connection::handleOut() { };
