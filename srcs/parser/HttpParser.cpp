#include "HttpParser.hpp"
#include "Request.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

using std::string;
using std::runtime_error;

const uchar* HttpParser::delimiters() {
	static unsigned char isDelimiter[256] = {0};
	isDelimiter[' '] = Token::SPACE;
	isDelimiter['\t'] = Token::SPACE;
	isDelimiter['\r'] = Token::NEWLINE;
	isDelimiter['\n'] = Token::NEWLINE;
	isDelimiter['#'] = Token::COMMENT;
	isDelimiter['"'] = Token::QUOTE;
	isDelimiter['{'] = Token::OPENBLOCK;
	isDelimiter['}'] = Token::CLOSEBLOCK;
	isDelimiter[';'] = Token::SEMICOLON;
	isDelimiter['\\'] = Token::EXCAPE;
	isDelimiter['\0'] = Token::ENDOFILE;
	return isDelimiter;
}

// Public constructors and destructors
HttpParser::HttpParser():
	_request(new Request),
	_charRead(0),
	_headerLen(0),
	_state(REQUEST_LINE),
	_token(delimiters(), _request->_headerBuff),
	_expect(_token)
{}

HttpParser::~HttpParser() {}

// Public Methods
void	HttpParser::receive(int fd) {
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

void	HttpParser::parseRequestLine()
{
	_token.next();
	_request->_method = _expect.method();

	_expect.path(&_request->_path);

	_token.next();
	if (!_token.compare("HTTP/1.1") && !_token.compare("HTTP/1.0"))
		throw _expect.parsingErr("HTTP/*");

	_token.getNextOfType(Token::NEWLINE, "NEWLINE");
	if (_token.compare("\r\n\r\n")) {
		_state = RETURN;
		return;
	}
	else if (!_token.compare("\r\n"))
		throw _expect.parsingErr("NEWLINE");
	_state = HEADERS;
}

void	HttpParser::parseHeaders()
{
	StrView	headerKey(_request->_headerBuff);
	StrView	headerValue(_request->_headerBuff);

	while (1) {
		_token.getNextOfType(Token::WORD, "Http header key or eof");
		headerKey = _token.getStrV();

		_token.getNextOfType(Token::WORD, "http header Value");
		headerValue = _token.getStrV();

		_token.getNextOfType(Token::NEWLINE, "NEWLINE");
		if (_token.compare("\r\n\r\n")) {
			_state = RETURN;
			return;
		}
		else if (!_token.compare("\r\n"))
			throw _expect.parsingErr("NEWLINE");
	}

}

Request* HttpParser::parse(const char *inBuff, size_t size) {
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
