#include "HttpParser.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <utility>
#include <vector>

using std::make_pair;
using std::runtime_error;
using std::string;

const char *g_methods[] = {"DEFAULT", "GET", "POST", "DELETE"};

const char *const HttpParser::bodyLabels[STATE_SIZE]
	= {"REQUEST_LINE",		  "HEADERS",	   "VALIDATE",	   "BODY",
	   "SET_CHUNK_SIZE",	  "SET_BODY_SIZE", "CHUNKED_BODY", "NO_BODY",
	   "MAKE_ERROR_RESPONSE", "RETURN"};

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
HttpParser::HttpParser(const Server &server) :
	_server(server),
	_request(new Request),
	_charRead(0),
	_headerLen(0),
	_state(REQUEST_LINE),
	_nextBodySection(0),
	_needsMoreInput(false),
	_toGetChunk(false),
	_token(delimiters(), _request->_headerBuff),
	_expect(_token),
	_status(200) {}

HttpParser::~HttpParser() {}

// Public Methods
uchar HttpParser::handleNewline() {
	_token.loadNextOfType(Token::NEWLINE, "NEWLINE");

	if (_token.compare("\r\n\r\n")) {
		_state = VALIDATE;
		return VALIDATE;
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
	try {
		_token.loadNextOfType(Token::WORD, "Http Method");
		const uchar method = _expect.method();
		if (Location::DEFAULT == method)
			return setError(400, "Http method non existent. ");
		_request->_method = method;

		_expect.consolidatedPath(&_request->_path);

		_token.loadNext();
		if (!_token.compare("HTTP/1.1")) {
			if (_token.compare("HTTP/1.0"))
				_request->_http1_1 = false;
			else
				return setError(505, "Http vertion not suported");
		}

		if (RETURN == handleNewline())
			return;

		_state = HEADERS;
	} catch (const runtime_error &err) {
		LOG_ERROR(err);
		setError(400, "Invalid http request line");
	}
}

void HttpParser::validateRequestLine() {
	const Location &location = _server.findLocation(_request->getPath());

	if (!location.isAllowedMethod(_request->getMethod()))
		setError(405, "Method not allowed");
}

void HttpParser::validateHeader() {}

void HttpParser::setError(const uint errorCode, const char *detailMsg) {
	_status = errorCode;
	_state = MAKE_ERROR_RESPONSE;
	LOGNUM_LABELED(Logger::WARNING, detailMsg,
				   " | Registered html error code: ", errorCode);
}

void HttpParser::validateKey(StrView Key) {
	switch (*Key.getStart()) {
	case ('H'):
		if (Key.compare("Host") && _request->getHeaderValue(Key))
			return setError(400, "Request has more than one host!");
		return;
	}
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

void HttpParser::validateRequest() {
	if (!_request->getHeaderValue("Host"))
		return setError(400, "Request has no Host!");
};

void HttpParser::setBodySize() {
	const StrView *bodyType = _request->getHeaderValue("Transfer-Encoding");
	if (bodyType && bodyType->compare("chunked")) {
		_state = SET_CHUNK_SIZE;
		return;
	}

	bodyType = _request->getHeaderValue("Content-Length");
	if (bodyType) {
		string size = bodyType->getStr();
		_nextBodySection = atoll(size.c_str());
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
		case (VALIDATE):
			validateRequest();
		case (SET_BODY_SIZE):
			setBodySize();
			LOGNUM_LABELED(Logger::LOG, "Body mode set to ", bodyLabels[_state],
						   _state);
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
