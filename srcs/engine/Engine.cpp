#include "Engine.hpp"
#include "ASocket.hpp"
#include "ConfParser.hpp"
#include "Listening.hpp"
#include <cerrno>
#include <stdint.h>
#include <cstring>
#include <map>
#include <netinet/in.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <utility>
#include <vector>

using std::runtime_error;
using std::map;
using std::vector;
using std::string;

// Public constructors and destructors
Engine::Engine():
	_fdEpoll(-1)
{
	epoll_init();
	// ASocket* newSocket = Listening::create(config);
	// addSocket(newSocket);
}

Engine::~Engine() {
	map<int, ASocket*>::iterator socket = _sockets.begin();
	while (socket != _sockets.end())
		delete (socket++)->second;

	vector<Server *>::iterator server = _servers.begin();
	while (server != _servers.end())
		delete *server++;

	if (_fdEpoll > 0)
		close(_fdEpoll);
}

// Error handeling
runtime_error	Engine::handleError(const string errMsg) {
	return runtime_error( errMsg + strerror(errno));
}

// Public Methods
void Engine::epoll_init() {
	_fdEpoll = epoll_create(1);
	if (_fdEpoll < 0)
		throw handleError("Error Epoll_create: ");
}

ASocket* Engine::getSocket(int fd) {
	map<int, ASocket*>::iterator socket = _sockets.find(fd);
	if (socket != _sockets.end())
		return socket->second;
	return NULL;
}

void	Engine::setEventTo(int epollFd, uint operation, uint eventType, int socketFd, void *ptrToSock) {
	struct epoll_event event;
	event.events = eventType;
	event.data.ptr = ptrToSock;
	if (OK == epoll_ctl(epollFd, operation, socketFd, &event))
		return;
	throw handleError("Error setting epoll socket event type: ");
}

void Engine::addSocket(ASocket* socket) {
	if (!socket)
		throw handleError("Error null socket");
	int fd = socket->getFd();
	if (socket && fd > 0) {
		_sockets[fd] = socket;
		setEventTo(_fdEpoll , EPOLL_CTL_ADD, EPOLLIN, fd, socket->getPtrToSelf());
	}
	else throw handleError("Error adding socket");
}

void Engine::deleteSocket(int fd) {
	map<int, ASocket*>::iterator socket = _sockets.find(fd);
	if (socket != _sockets.end()) {
		setEventTo(_fdEpoll, EPOLL_CTL_DEL, 0, fd, socket->second->getPtrToSelf());
		delete socket->second;
		_sockets.erase(socket);
	}
}

void Engine::buildServers(string& config) {
	ConfParser parser(config, _servers);
	parser.createServers();
}

void Engine::createSockets() {
	vector<Server *>::iterator server = _servers.begin();
	while (server != _servers.end()) {
		vector<Listen>::iterator port = (*server)->_listen.begin();
		while (port != (*server)->_listen.end()) {
			Listening* socket = Listening::create(**server, *port);
			_sockets.insert(std::make_pair(socket->getFd(), socket));
			port++;
		}
		server++;
	}
}

void Engine::run(string& config) {
	buildServers(config);
	createSockets();
	while (1) {
		
	}
}









