#include "Request.hpp"
#include "StrView.hpp"
#include <string>

using std::string;

// Public constructors and destructors
Request::Request():
	_method(0),
	_path(_headerBuff)
{
	_headerBuff.reserve(MAX_HEADER_SIZE);
}

Request::~Request() {}

// Public Methods
uchar			Request::getMethod() const { return _method; };
const StrView&	Request::getPath() const { return _path; };
