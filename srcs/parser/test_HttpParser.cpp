#include "HttpParser.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <gtest/gtest.h>
#include <string>
#include <sys/types.h>

using std::string;

class HttpParserTest : public ::testing::Test {
private:
	Request *_request;
	HttpParser parser;

protected:
	void TearDown() {
		if (_request)
			delete _request;
		_request = NULL;
	}

public:
	HttpParserTest() :
		_request(NULL) {}

	void testRequestLine(const char *testStr, uchar expectedMethod,
						 const char *expectedPath) {
		std::string str = testStr;
		_request = parser.parse(str.c_str(), str.length());
		EXPECT_TRUE(_request);
		EXPECT_EQ(_request->getMethod(), expectedMethod);
		EXPECT_EQ(_request->getPath(), expectedPath);
	}

	string buildHeaders(const char *headers[][2]) {
		std::stringstream ss;
		for (size_t i = 0; headers[i][0] != NULL; ++i) {
			ss << headers[i][0] << ": " << headers[i][1] << "\r\n";
		}
		ss << "\r\n";
		return ss.str();
	}

	string buildRequest(const string &requestLine, const char *headers[][2]) {
		std::stringstream ss;
		ss << requestLine << "\r\n";
		ss << buildHeaders(headers);
		return ss.str();
	}

	void testParseHeaders(const char *expectedHeaders[][2]) {
		string requestStr = buildHeaders(expectedHeaders);
		parser._state = HttpParser::HEADERS;
		_request = parser.parse(requestStr.c_str(), requestStr.length());

		ASSERT_TRUE(_request);

		for (size_t i = 0; expectedHeaders[i][0]; ++i) {
			const char *expKey = expectedHeaders[i][0];
			const char *expValue = expectedHeaders[i][1];
			const StrView *value = _request->getHeaderValue(expKey);
			EXPECT_TRUE(value)
				<< "Key " << expKey << " does not exist in headers map";
			EXPECT_STREQ(value->getStr().c_str(), expValue);
		}
	}
};

TEST_F(HttpParserTest, BasicTestGet) {
	testRequestLine("GET /folder HTTP/1.1\r\n\r\n", Location::GET, "/folder");
}

TEST_F(HttpParserTest, BasicTestPost) {
	testRequestLine("POST /folder HTTP/1.1\r\n\r\n", Location::POST, "/folder");
}

TEST_F(HttpParserTest, BasicTestDelete) {
	testRequestLine("DELETE /folder HTTP/1.1\r\n\r\n", Location::DELETE,
					"/folder");
}

TEST_F(HttpParserTest, HeadersTest) {
	const char *headers[][2] = {
		{"Host", "localhost:8080"},
		{"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:148.0) "
					   "Gecko/20100101 Firefox/148.0"},
		{"Accept",
		 "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
		{"Accept-Language", "en-US,en;q=0.9"},
		{"Accept-Encoding", "gzip, deflate, br, zstd"},
		{"Connection", "keep-alive"},
		{"Upgrade-Insecure-Requests", "1"},
		{"Sec-Fetch-Dest", "document"},
		{"Sec-Fetch-Mode", "navigate"},
		{"Sec-Fetch-Site", "none"},
		{"Sec-Fetch-User", "?1"},
		{"DNT", "1"},
		{"Sec-GPC", "1"},
		{"Priority", "u=0, i"},
		{NULL, NULL}};

	testParseHeaders(headers);
}

TEST_F(HttpParserTest, HeadersTestSubset) {
	const char *headers[][2] = {{"Host", "example.com"},
								{"Content-Type", "application/json"},
								{"Content-Length", "42"},
								{NULL, NULL}};

	testParseHeaders(headers);
}
