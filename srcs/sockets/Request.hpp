#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "StrView.hpp"
#include "webServ.hpp"
#include <map>
#include <string>
#include <sys/types.h>

#define MAX_HEADER_SIZE 8192

class Request {
private:
	std::string					_headerBuff;

	uchar						_method;
	StrView						_path;

	std::map<StrView, StrView>	headers;

	friend class				HttpParser;

public:
	// Constructors and destructors
	Request();
	Request(const Request& other);
	~Request();

	// Operators overload
	Request& operator=(const Request& other);

	// Getters and setters

	// Methods
	const StrView&	getPath() const;
	uchar			getMethod() const;

};

#endif

