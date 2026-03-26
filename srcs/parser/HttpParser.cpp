#include "HttpParser.hpp"
#include "Request.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <cstdlib>
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
	_nextBodySection(0),
	_needsMoreInput(false),
	_toGetChunk(false),
	_token(delimiters(), _request->_headerBuff),
	_expect(_token) {}

HttpParser::~HttpParser() {}

// Public Methods
uchar HttpParser::handleNewline() {
	_token.loadNextOfType(Token::NEWLINE, "NEWLINE");

	if (_token.compare("\r\n\r\n")) {
		_state = RETURN;
		return RETURN;
	} else if (_token.compare("\r\n"))
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
		if (RETURN == handleNewline()) {
			_state = SET_BODY_SIZE;
			return;
		}
	}
}

void HttpParser::setBodySize() {
	const StrView *bodyType = _request->getHeaderValue("Transfer-Encoding");
	if (bodyType && bodyType->compare("chunked")) {
		_state = CHUNKED_BODY;
		return;
	}

	bodyType = _request->getHeaderValue("Content-Length");
	if (bodyType) {
		string size = bodyType->getStr();
		_nextBodySection = atoll(size.c_str());
		size_t headerSize = _token.getEnd() - _token.getStart();
		_request->_headerBuff.reserve(headerSize + _nextBodySection);
		_state = BODY;
		return;
	}

	_state = RETURN;
}

void HttpParser::getChunk() {
	if (_token.getSizeLeft() < _nextBodySection) {
		_nextBodySection -= _token.getSizeLeft();
		_chunks.push_back(_token.getRemaining());
		_needsMoreInput = true;
		return;
	}

	_token.loadNextChunk(_nextBodySection);
	_nextBodySection = 0;
	_chunks.push_back(_token.getStrV());

	switch (_token.loadHttpNewLine()) {
	case (Token::ENDOFILE):
		_state = RETURN;
		return;
	case (Token::OTHER):
		_needsMoreInput = true;
	case (Token::NEWLINE):
		_state = SET_BODY_SIZE;
	}
}

Request *HttpParser::parse(const char *inBuff, size_t size) {
	_needsMoreInput = false;
	_headerLen += size;
	_request->_headerBuff.append(inBuff, size);

	while (!_needsMoreInput) {
		switch (_state) {
		case (REQUEST_LINE):
			parseRequestLine();
			continue;
		case (HEADERS):
			parseHeaders();
			continue;
		case (SET_BODY_SIZE):
			setBodySize();
			continue;
		case (BODY):
			if (_token.getSizeLeft() < _nextBodySection) {
				_needsMoreInput = true;
				continue;
			}
			_request->_body = _token.getRemaining();
			_state = RETURN;
			continue;
		case (SET_CHUNK_SIZE):
			_needsMoreInput = _token.loadNextHex(&_nextBodySection);
			continue;
		case (CHUNKED_BODY):
			getChunk();
			continue;
		case (RETURN):
			Request *ret = _request;
			_request = new Request;
			return ret;
		}
	}
	return NULL;
}
