#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "Expect.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Token.hpp"
#include <linux/stat.h>
#include <sys/types.h>
#include <vector>

#define BUFFER_SIZE 1024
#define NEEDS_MORE_INPUT true

class Server;

class HttpParser {
private:
	enum state {
		REQUEST_LINE,
		HEADERS,
		VALIDATE,
		BODY,
		SET_CHUNK_SIZE,
		SET_BODY_SIZE,
		CHUNKED_BODY,
		NO_BODY,
		MAKE_ERROR_RESPONSE,
		RETURN,
		STATE_SIZE
	};

	static const char *const bodyLabels[STATE_SIZE];

	const Server &_server;
	Request *_request;
	Response *_response;

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

	uint _status;

	// Methods
	void validateRequestLine();
	void setError(const uint errorCode, const char *detailMsg);
	void validateRequest();
	void validateKey(StrView Key);
	void validateHeader();
	void getChunk();
	void setBodySize();
	uchar handleNewline();
	void receive(int fd);
	void parseHeaders();
	void parseRequestLine();

	// Static initializer for Token class
	static const uchar *delimiters();
	// Explicit disables
	HttpParser();
	HttpParser &operator=(const HttpParser &other);
	HttpParser(const HttpParser &other);

	friend class HttpParserTest;

public:
	// Constructors and destructors
	HttpParser(const Server &server);
	~HttpParser();

	// Operators overload

	// Getters and setters

	// Methods
	Request *parse(const char *inBuff, size_t size);
};

#endif
