#include "Request.hpp"
#include "Location.hpp"
#include "StrView.hpp"
#include <cstring>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>

using std::cout;
using std::map;
using std::string;
using std::stringstream;

// Public constructors and destructors
Request::Request() :
	_method(Location::DEFAULT),
	_path(_headerBuff),
	_query(_headerBuff),
	_fragment(_headerBuff),
	_http1_1(true),
	_body(_headerBuff) {
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

const char *Request::safeStr(const char *str) const {
	return str ? str : "NULL";
}

void Request::print(stringstream &stream) const {
	stream << "Method: " << _method;
	stream << " | Path: '" << safeStr(_path.getStart()) << "'|'"
		   << safeStr(_query.getStart()) << "'|'"
		   << safeStr(_fragment.getStart()) << "'"
		   << "\n";

	map<StrView, StrView>::const_iterator header = _headers.begin();
	for (; header != _headers.end(); header++)
		stream << safeStr(header->first.getStart()) << ": "
			   << safeStr(header->second.getStart()) << "\n";
}

const string &Request::getBody() const { return _body; }
