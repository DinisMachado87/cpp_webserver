#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <sys/types.h>

#define MAX_HEADER_SIZE 8192

class Request {
private:
	std::string _headerBuff; // StrView Buffer
	std::map<StrView, StrView> _headers;
	uchar _method;
	StrView _path;
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
	const StrView &getBody() const;
	void print();

	// Getters and setters
	const StrView *getHeaderValue(const char *charKey) const;
	const StrView *getHeaderValue(StrView &key) const;
	const StrView &getPath() const;
	uchar getMethod() const;
};

#endif
