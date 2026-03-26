#include "HttpParser.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <ostream>
#include <string>
#include <sys/types.h>

using std::endl;
using std::string;

class HttpParserTest : public ::testing::Test {
private:
	string _requestStr;
	string _body;
	Request *_request;
	HttpParser _parser;

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
		_request = _parser.parse(str.c_str(), str.length());
		EXPECT_TRUE(_request);
		EXPECT_EQ(_request->getMethod(), expectedMethod);
		EXPECT_EQ(_request->getPath(), expectedPath);
	}

	void buildHeaders(const char *headers[][2]) {
		std::stringstream ss;
		for (size_t i = 0; headers[i][0] != NULL; ++i) {
			ss << headers[i][0] << ": " << headers[i][1] << "\r\n";
		}
		ss << "\r\n";
		_requestStr.append(ss.str());
	}

	void ParseHeaders() {
		_parser._state = HttpParser::HEADERS;
		_request = _parser.parse(_requestStr.c_str(), _requestStr.length());
	}

	void appendBody(size_t size) {
		const string strTest = "test_";
		const size_t strTestLen = 5;
		size_t rounds = size / strTestLen;
		size_t leftover = size % strTestLen;
		while (rounds--)
			_body.append(strTest);
		_body.append(strTest, leftover);
		_requestStr.append(_body);
	}

	void testHeaders(const char *expectedHeaders[][2]) {
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

	void testBody(size_t expBodySize) {
		EXPECT_TRUE(_parser._nextBodySection == expBodySize);
		const StrView &body = _request->getBody();
		EXPECT_TRUE(body.ncompare(_body.c_str(), _body.length()))
			<< body.getStr() << "\n\n"
			<< _body << endl;
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

	buildHeaders(headers);
	ParseHeaders();
	testHeaders(headers);
}

TEST_F(HttpParserTest, HeadersTestSubset) {
	const char *headers[][2] = {{"Host", "example.com"},
								{"Content-Type", "application/json"},
								{"Content-Length", "42"},
								{NULL, NULL}};

	buildHeaders(headers);
	appendBody(42);
	ParseHeaders();
	testHeaders(headers);
	testBody(42);
}
