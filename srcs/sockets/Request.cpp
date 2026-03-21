#include "Request.hpp"
#include "StrView.hpp"
#include <cstring>
#include <map>
#include <string>

using std::map;
using std::string;

// Public constructors and destructors
Request::Request() :
	_method(0),
	_path(_headerBuff) {
	_headerBuff.reserve(MAX_HEADER_SIZE);
}

Request::~Request() {}

// Public Methods
uchar Request::getMethod() const { return _method; };
const StrView &Request::getPath() const { return _path; };

const StrView *Request::getHeaderValue(StrView &key) const {
	map<StrView, StrView>::const_iterator pair = _headers.find(key);
	if (pair == _headers.end())
		return NULL;
	return &pair->second;
};

const StrView *Request::getHeaderValue(const char *charKey) const {
	string key = charKey;
	StrView strViewKey = StrView(key, 0, key.length());
	return (getHeaderValue(strViewKey));
}
