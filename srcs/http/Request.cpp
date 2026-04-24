#include "Request.hpp"
#include "Location.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cstring>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>

using std::map;
using std::string;
using std::stringstream;

// Public constructors and destructors
Request::Request() :
	_path(_headerBuff),
	_query(_headerBuff),
	_fragment(_headerBuff),
	_method(Location::DEFAULT),
	_http1_1(true),
	_body(_headerBuff) {
	_headerBuff.reserve(MAX_HEADER_SIZE);
}

Request::~Request() {}

// Public Methods

uchar Request::getMethod() const { return _method; };
const char *Request::getMethodStr() const { return g_methods[_method]; };
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

void Request::print(std::ostream &stream) const {
	stream << "Method: " << getMethodStr();
	stream << " | Path: '" << _path << "'|'" << _query << "'|'" << _fragment
		   << "'\n";

	map<StrView, StrView>::const_iterator header = _headers.begin();
	for (; header != _headers.end(); header++) {
		stream << header->first << ": " << header->second << "\n";
	}
}

std::ostream &operator<<(std::ostream &os, const Request &req) {
	req.print(os);
	return os;
}

const StrView &Request::getBody() const { return _body; }
