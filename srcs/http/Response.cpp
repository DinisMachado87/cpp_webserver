#include "Response.hpp"
#include "webServ.hpp"

// Public constructors and destructors
Response::Response(const Request *request, const Location &location) :
	_request(request),
	_location(location) {}

Response::~Response() { delete _request; }

// Response::Response(const Response &other) { (void)other; }

Response &Response::operator=(const Response &other) {
	if (this == &other)
		return *this;
	return *this;
}

// Public Methods
int Response::send() { return DONE; }
