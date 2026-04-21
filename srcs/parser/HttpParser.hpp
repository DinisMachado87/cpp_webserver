#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "Expect.hpp"
#include "Request.hpp"
#include "Token.hpp"
#include <sys/types.h>
#include <vector>

#define BUFFER_SIZE 1024
#define NEEDS_MORE_INPUT true

class HttpParser {
private:
	static const char *const bodyLabels[8];

	enum state {
		REQUEST_LINE,
		HEADERS,
		BODY,
		SET_CHUNK_SIZE,
		SET_BODY_SIZE,
		CHUNKED_BODY,
		NO_BODY,
		RETURN
	};

	Request *_request;
	ssize_t _charRead;
	ssize_t _headerLen;
	uint _state;
	size_t _nextBodySection;
	bool _needsMoreInput;

	bool _toGetChunk;
	std::vector<StrView> _chunks;

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
	void getChunk();
	void setBodySize();
	uchar handleNewline();
	void receive(int fd);
	void parseHeaders();
	void parseRequestLine();
	Request *parse(const char *inBuff, size_t size);
};

#endif
