#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "StrView.hpp"
#include "webServ.hpp"
#include <map>
#include <string>
#include <sys/types.h>

class Request {
private:
	std::string _headerBuff;

	uint _method;

	StrView _path;
	StrView _query;
	StrView _fragment;

	bool _http1_1;

	std::map<StrView, StrView> _headers;
	std::string _body;

	friend class HttpParser;

public:
	// Constructors and destructors
	Request();
	Request(const Request &other);
	~Request();

	// Operators overload
	Request &operator=(const Request &other);

	// Methods
	const char *safeStr(const char *str) const;
	const std::string &getBody() const;
	void print(std::stringstream &stream) const;

	// Getters and setters
	const StrView *getHeaderValue(const char *charKey) const;
	const StrView *getHeaderValue(StrView &key) const;
	const StrView &getPath() const;
	uchar getMethod() const;
};

#endif
