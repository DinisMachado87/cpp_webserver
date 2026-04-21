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
	StrView _body;

	friend class HttpParser;

public:
	// Constructors and destructors
	Request();
	Request(const Request &other);
	~Request();

	// Operators overload
	Request &operator=(const Request &other);

	// Methods
	const char *getMethodStr() const;
	void print(std::ostream &stream) const;

	// Getters and setters
	const StrView *getHeaderValue(const char *charKey) const;
	const StrView *getHeaderValue(StrView &key) const;
	const StrView &getPath() const;
	uchar getMethod() const;
	const StrView &getBody() const;
};

std::ostream &operator<<(std::ostream &os, const Request &req);

#endif
