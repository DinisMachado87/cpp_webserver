#include "ConfParser.hpp"
#include "Expect.hpp"
#include "HttpParser.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <gtest/gtest.h>
#include <string>
#include <sys/types.h>

class HttpParserTest: public ::testing::Test {
private:
	Request*	_request;
	HttpParser	parser;

	void	tearDown() {
		if (_request)
			delete _request;
	}

public:
	void	testRequestLine(const char* testStr, uchar expectedMethod, const char* expectedPath) {
	std::string str = testStr;
	_request = parser.parse(str.c_str(), str.length());
	EXPECT_TRUE(_request);
	EXPECT_EQ(_request->getMethod(), expectedMethod);
	EXPECT_TRUE(_request->getPath().compare(expectedPath));
	}
};

TEST_F(HttpParserTest, BasicTestGet) {
	testRequestLine("GET /folder HTTP/1.1\r\n\r\n", Location::GET, "/folder");
}

TEST_F(HttpParserTest, BasicTestPost) {
	testRequestLine("POST /folder HTTP/1.1\r\n\r\n", Location::POST, "/folder");
}

TEST_F(HttpParserTest, BasicTestDelete) {
	testRequestLine("DELETE /folder HTTP/1.1\r\n\r\n", Location::DELETE, "/folder");
}
