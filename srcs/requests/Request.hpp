#pragma once

#include <string>


struct Location;

enum requestMethod : unsigned char
{
	GET,
	POST,
	DELETE,
};

							//RFC2616
typedef	struct	reqVars	{	//{defaults}
requestMethod	method;		//GET / POST / PUT / DELETE etc
int		contentLength;		//length of message body - taken from header or manually calculated if chunked {-1}
char*	CONTENT_TYPE;		//media type of message body - from header {NULL}
char*	PATH_INFO;			//path after the file name as presented in URL {NULL} e.g. URL=example.com/cgi-bin/hello.cgi/user/admin PATH_INFO=/user/admin
char*	QUERY_STRING;		//information for the CGI script to affect the return value - URL after '?' {NULL} e.g. URL=example.com/cgi-bin/hello.cgi/user/admin?query=date QUERY_STRING=query=date
char*	REMOTE_ADDR;		//network address of client sending the request (ipv4 or ipv6) {NULL}
char*	REMOTE_HOST;		//domain name of the client sending the request, or {NULL}
char*	SCRIPT_NAME;		//path and name of the .cgi file to be run {NULL} e.g. URL=example.com/cgi-bin/hello.cgi/user/admin SCRIPT_NAME=/cgi-bin/hello.cgi
}	reqVars;


class	Request
{
public:
	Request(Location* loc);
	~Request(void);


protected:
	std::string	_body;
	reqVars		_vars;
	Location*	_location;

private:
	Request(void);
	Request(const Request &other);
	Request &	operator=(const Request &other);
};
