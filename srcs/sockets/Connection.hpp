#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "ASocket.hpp"
#include "HttpParser.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <sys/epoll.h>

class Connection : public ASocket {
private:
	enum _handleInState { REQUEST, RESPONSE, INITBODY, LOOPBODY };
	HttpParser _http;
	Response *_responses[RESPONSES_CUE_SIZE];
	Response *_responseReceivingBody;
	size_t _cur;
	size_t _back;
	uchar _handleInState;

	// Explicit disables
	Connection(const int fd, const Server &server,
			   struct sockaddr_in serverAddr);
	Connection(const Connection &other);
	Connection &operator=(const Connection &other);
	// Methods
	// Friends
	friend class Listening;

public:
	// Constructors and destructors
	~Connection();
	// I/O
	Connection *handleIn();
	void handleOut();
	// Event tracking
	uint32_t getEventsNextLoop();
	bool isFull() const;
	ssize_t recvToBuffer(char *buffer);
};

#endif
