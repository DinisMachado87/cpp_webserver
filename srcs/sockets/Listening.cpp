#include "Listening.hpp"
#include "ASocket.hpp"
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
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

using std::string;
using std::runtime_error;

runtime_error	Listening::handleFdError(int fdSock)
{
	if (0 < fdSock)
		close(fdSock);
	return runtime_error(
		string("Error creating Listening Socket: ")
		+ strerror(errno));
}

// Public constructors and destructors
Listening::Listening(int fd, const Server& server, struct sockaddr_in serverAddr):
	ASocket(fd, server, serverAddr) {}

Listening::~Listening() {}

// Public Methods
Listening* Listening::create(const Server& server, const Listen& listenSock)
{
	struct sockaddr_in	addr = {
		AF_INET,
		htons(listenSock.getPort()),
		{listenSock.getHost()},
		{0}
	};

	int	fdSock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

	int enable = 1;

	if (0 <= fdSock
		&& OK == setNonBlocking(fdSock)
		&& OK == setsockopt(fdSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))
		&& OK == bind(fdSock, (struct sockaddr*)&addr, sizeof(addr))
		&& OK == listen(fdSock, SOMAXCONN))
		return new Listening(fdSock, server, addr);
	throw handleFdError(fdSock);
}

Connection* Listening::handleIn() {
	struct sockaddr_in	clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	int clientFd = accept(_fd, (sockaddr*)&clientAddr, &clientAddrLen);
	if (OK <= clientFd)
		throw handleError("Error accepting client: ");

	setNonBlocking(clientFd);

	return new Connection(clientFd, _server, clientAddr);
}

void Listening::handleOut() { }

