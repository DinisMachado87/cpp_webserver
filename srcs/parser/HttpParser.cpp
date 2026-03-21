#include "HttpParser.hpp"
#include "Request.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <utility>

using std::make_pair;
using std::runtime_error;
using std::string;

const uchar *HttpParser::delimiters() {
	static uchar isDelimiter[256] = {0};
	isDelimiter[' '] = Token::SPACE;
	isDelimiter['\t'] = Token::SPACE;
	isDelimiter['\r'] = Token::NEWLINE;
	isDelimiter['\n'] = Token::NEWLINE;
	isDelimiter['\0'] = Token::ENDOFILE;
	return isDelimiter;
}

// Public constructors and destructors
HttpParser::HttpParser() :
	_request(new Request),
	_charRead(0),
	_headerLen(0),
	_state(REQUEST_LINE),
	_token(delimiters(), _request->_headerBuff),
	_expect(_token) {}

HttpParser::~HttpParser() {}

// Public Methods
uchar HttpParser::handleNewline() {
	_token.loadNextOfType(Token::NEWLINE, "NEWLINE");

	if (_token.compare("\r\n\r\n")) {
		_state = RETURN;
		return RETURN;
	}

	else if (_token.compare("\r\n"))
		return NEEDS_MORE_INPUT;

	throw _expect.parsingErr("NEWLINE");
}

void HttpParser::receive(int fd) {
	size_t leftInBuff = MAX_HEADER_SIZE - _headerLen;
	if (leftInBuff <= 0)
		throw runtime_error("Max header size exceeded");

	size_t maxToRead = BUFFER_SIZE < leftInBuff ? BUFFER_SIZE : leftInBuff;

	_charRead = recv(fd, &_buff, maxToRead, 0);
	if (_charRead == ERR)
		throw runtime_error("Error reading http header: invalid recv()");

	_headerLen += _charRead;
	_request->_headerBuff.append(_buff, _charRead);
}

void HttpParser::parseRequestLine() {
	_token.loadNext();
	_request->_method = _expect.method();

	_expect.path(&_request->_path);

	_token.loadNext();
	if (!_token.compare("HTTP/1.1") && !_token.compare("HTTP/1.0"))
		throw _expect.parsingErr("HTTP/*");

	if (RETURN == handleNewline())
		return;

	_state = HEADERS;
}

void HttpParser::parseHeaders() {
	StrView key(_request->_headerBuff);
	StrView value(_request->_headerBuff);

	while (1) {
		_token.loadNextOfType(Token::WORD, "Http header key or eof");
		key = _token.getStrV();

		if (*key.getEnd() != ':')
			throw _expect.parsingErr(":");
		key.trimEnd(1);

		_token.loadNextStr("http header Value");
		value = _token.getStrV();

		_request->_headers.insert(make_pair(key, value));
		if (RETURN == handleNewline())
			return;
	}
}

Request *HttpParser::parse(const char *inBuff, size_t size) {
	_headerLen += size;
	_request->_headerBuff.append(inBuff, size);

	while (1) {
		switch (_state) {
		case (REQUEST_LINE):
			parseRequestLine();
			continue;
		case (HEADERS):
			parseHeaders();
			continue;
		case (RETURN):
			Request *ret = _request;
			_request = new Request;
			return ret;
		}
	}
	return NULL;
}
