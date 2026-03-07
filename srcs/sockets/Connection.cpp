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

runtime_error	handleError() {
	return runtime_error(string("Error creating Socket: ") + strerror(errno));
}

// Public constructors and destructors
Connection::Connection(int fd, Server& server):
	ASocket(fd, server) {}

Connection::~Connection() {}

// Public Methods
Connection*	Connection::create(Server& server, Listen& listenSock) {
	(void)server;
	(void)listenSock;
	return NULL;
};

void Connection::handle(int events) {
	(void)events;
};
