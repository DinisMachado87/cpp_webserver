#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "Expect.hpp"
#include "Request.hpp"
#include "Token.hpp"
#include <sys/types.h>

#define BUFFER_SIZE 1024

class HttpParser {
private:
	enum { NEEDS_MORE_INPUT, REQUEST_LINE, HEADERS, BODY, RETURN };

	Request *_request;
	ssize_t _charRead;
	ssize_t _headerLen;
	uint _state;

	Token _token;
	Expect _expect;
	char _buff[BUFFER_SIZE];

	// Static initializer
	static const uchar *delimiters();
	// Explicit disables
	HttpParser &operator=(const HttpParser &other);
	HttpParser(const HttpParser &other);

	friend class HttpParserTest;

public:
	// Constructors and destructors
	HttpParser();
	~HttpParser();

	// Operators overload

	// Getters and setters

	// Methods
	uchar handleNewline();
	void receive(int fd);
	void parseHeaders();
	void parseRequestLine();
	Request *parse(const char *inBuff, size_t size);
};

#endif
