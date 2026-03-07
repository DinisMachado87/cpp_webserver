// Tester
#include <gtest/gtest.h>
// Other classes in the project
#include "ConfParser.hpp"
#include "Server.hpp"
#include "debug.hpp"
// Library imports
#include <string>
#include <vector>
#include <stdexcept>

#define RESET   "\033[0m"
#define GREEN   "\033[1;32m"
#define RED     "\033[1;31m"

class ConfParserTest: public ::testing::Test {
protected:
	std::vector<Server*>	_servers;
	std::string				_config;

	enum MethodBits {
		BIT_GET = 1,
		BIT_POST = 2,
		BIT_PUT = 3,
		BIT_DELETE = 4
	};

	void TearDown() {
		for (size_t i = 0; i < _servers.size(); i++)
			delete _servers[i];
		_servers.clear();
	}

	void parse(std::string config) {
		_config = config;
		ConfParser parser(_config, _servers);
		parser.createServers();
	}

	std::string strv(const StrView& server) {
		return std::string(server.getStart(), server.getLen());
	}

	Server* server(size_t idx = 0) {
		EXPECT_LT(idx, _servers.size());
		return _servers[idx];
	}

	Location& location(size_t serverIdx, size_t locIdx) {
		EXPECT_LT(locIdx, server(serverIdx)->_locations.size());
		return server(serverIdx)->_locations[locIdx];
	}

	void assertSingleServer() {
		ASSERT_EQ(_servers.size(), 1);
	}

	bool hasMethod(const Location& loc, int methodBit) {
		return (loc._allowedMethods & (1 << methodBit)) != 0;
	}
};

TEST_F(ConfParserTest, EmptyServer) {
	parse("server { }");

	assertSingleServer();
	EXPECT_EQ(server()->getListenLen(), 0);
	EXPECT_EQ(server()->getLoncationsLen(), 0);
}

TEST_F(ConfParserTest, BasicServer) {
	parse("server { listen 127 8080; root /var/www; }");

	assertSingleServer();
	ASSERT_EQ(server()->getListenLen(), 1);
	EXPECT_EQ(server()->_listen[0]._host, 127);
	EXPECT_EQ(server()->_listen[0]._port, 8080);
	EXPECT_EQ(strv(server()->_defaults._root), "/var/www");
}

TEST_F(ConfParserTest, BasicServerNoHost) {
	parse("server { listen 8080; root /var/www; }");

	assertSingleServer();
	ASSERT_EQ(server()->getListenLen(), 1);
	EXPECT_EQ(server()->_listen[0]._host, 0);
	EXPECT_EQ(server()->_listen[0]._port, 8080);
	EXPECT_EQ(strv(server()->_defaults._root), "/var/www");
}

TEST_F(ConfParserTest, ServerWithIndexAndAutoindex) {
	parse(
		"server {\n"
		"  index index.html index.htm default.html;\n"
		"  autoindexing on;\n"
		"}"
	);

	assertSingleServer();
	EXPECT_EQ(server()->_defaults._index.len(), 3);
	EXPECT_EQ(strv(server()->_defaults._index[0]), "index.html");
	EXPECT_EQ(strv(server()->_defaults._index[1]), "index.htm");
	EXPECT_EQ(strv(server()->_defaults._index[2]), "default.html");
	EXPECT_TRUE(server()->_defaults._autoindex);
}

TEST_F(ConfParserTest, ClientMaxBodySize) {
	parse(
		"server { client_max_body_size 1024; }\n"
		"server { client_max_body_size 5k; }\n"
		"server { client_max_body_size 10m; }\n"
		"server { client_max_body_size 2g; }"
	);

	ASSERT_EQ(_servers.size(), 4);
	EXPECT_EQ(server(0)->_defaults._clientMaxBody, 1024);
	EXPECT_EQ(server(1)->_defaults._clientMaxBody, 5 * 1024);
	EXPECT_EQ(server(2)->_defaults._clientMaxBody, 10 * 1024 * 1024);
	EXPECT_EQ(server(3)->_defaults._clientMaxBody, 2UL * 1024 * 1024 * 1024);
}

TEST_F(ConfParserTest, ErrorPages) {
	parse("server { error_page 404 /404.html; error_page 500 /500.html; error_page 403 /forbidden.html; }");

	assertSingleServer();
	EXPECT_STREQ(server()->_defaults.findErrorFile(404), "/404.html");
	EXPECT_STREQ(server()->_defaults.findErrorFile(500), "/500.html");
	EXPECT_STREQ(server()->_defaults.findErrorFile(403), "/forbidden.html");
}

TEST_F(ConfParserTest, LocationBasic) {
	parse(
		"server {\n"
		"  location / { }\n"
		"  location /api { }\n"
		"  location /upload { }\n"
		"}"
	);

	assertSingleServer();
	ASSERT_EQ(server()->_locations.size(), 3);
	EXPECT_STREQ(location(0, 0).getPath(), "/");
	EXPECT_STREQ(location(0, 1).getPath(), "/api");
	EXPECT_STREQ(location(0, 2).getPath(), "/upload");
}

TEST_F(ConfParserTest, LocationAllowedMethods) {
	parse(
		"server {\n"
		"  location / { allowed_methods GET POST; }\n"
		"  location /api { allowed_methods GET POST PUT DELETE; }\n"
		"}"
	);

	assertSingleServer();
	ASSERT_EQ(server()->_locations.size(), 2);

	EXPECT_TRUE(hasMethod(location(0, 0), BIT_GET));
	EXPECT_TRUE(hasMethod(location(0, 0), BIT_POST));
	EXPECT_FALSE(hasMethod(location(0, 0), BIT_PUT));
	EXPECT_FALSE(hasMethod(location(0, 0), BIT_DELETE));

	EXPECT_TRUE(hasMethod(location(0, 1), BIT_GET));
	EXPECT_TRUE(hasMethod(location(0, 1), BIT_POST));
	EXPECT_TRUE(hasMethod(location(0, 1), BIT_PUT));
	EXPECT_TRUE(hasMethod(location(0, 1), BIT_DELETE));
}

TEST_F(ConfParserTest, LocationReturn) {
	parse(
		"server {\n"
		"  location /old { return 301 /new; }\n"
		"  location /gone { return 410 /error; }\n"
		"}"
	);

	assertSingleServer();
	ASSERT_EQ(server()->_locations.size(), 2);
	EXPECT_EQ(location(0, 0)._returnCode, 301);
	EXPECT_EQ(strv(location(0, 0)._returnPath), "/new");
	EXPECT_EQ(location(0, 1)._returnCode, 410);
	EXPECT_EQ(strv(location(0, 1)._returnPath), "/error");
}

TEST_F(ConfParserTest, LocationUpload) {
	parse(
		"server {\n"
		"  location /upload {\n"
		"    upload_enable on;\n"
		"    upload_path /var/www/uploads;\n"
		"  }\n"
		"}"
	);

	assertSingleServer();
	ASSERT_EQ(server()->_locations.size(), 1);
	EXPECT_TRUE(location(0, 0)._uploadEnable);
	EXPECT_EQ(strv(location(0, 0)._uploadPath), "/var/www/uploads");
}

TEST_F(ConfParserTest, LocationCGI) {
	parse(
		"server {\n"
		"  location /cgi-bin {\n"
		"    cgi_extension .py .php .pl;\n"
		"    cgi_path /usr/bin/python3 /usr/bin/php /usr/bin/ruby;\n"
		"  }\n"
		"}"
	);

	assertSingleServer();
	ASSERT_EQ(server()->_locations.size(), 1);
	EXPECT_EQ(location(0, 0)._cgiExtensions.len(), 3);
	EXPECT_EQ(strv(location(0, 0)._cgiExtensions[0]), ".py");
	EXPECT_EQ(strv(location(0, 0)._cgiExtensions[1]), ".php");
	EXPECT_EQ(strv(location(0, 0)._cgiExtensions[2]), ".pl");
	EXPECT_EQ(location(0, 0)._cgiPath.len(), 3);
	EXPECT_EQ(strv(location(0, 0)._cgiPath[0]), "/usr/bin/python3");
	EXPECT_EQ(strv(location(0, 0)._cgiPath[1]), "/usr/bin/php");
	EXPECT_EQ(strv(location(0, 0)._cgiPath[2]), "/usr/bin/ruby");
}

TEST_F(ConfParserTest, LocationOverrides) {
	parse(
		"server {\n"
		"  root /var/www/html;\n"
		"  autoindexing off;\n"
		"  client_max_body_size 1m;\n"
		"  location /special {\n"
		"    root /var/www/special;\n"
		"    autoindexing on;\n"
		"    client_max_body_size 5m;\n"
		"  }\n"
		"}"
	);

	assertSingleServer();
	EXPECT_EQ(strv(server()->_defaults._root), "/var/www/html");
	EXPECT_FALSE(server()->_defaults._autoindex);
	EXPECT_EQ(server()->_defaults._clientMaxBody, 1024 * 1024);

	ASSERT_EQ(server()->_locations.size(), 1);
	EXPECT_EQ(strv(location(0, 0)._overrides._root), "/var/www/special");
	EXPECT_TRUE(location(0, 0)._overrides._autoindex);
	EXPECT_EQ(location(0, 0)._overrides._clientMaxBody, 5 * 1024 * 1024);
}

TEST_F(ConfParserTest, CompleteConfiguration) {
	parse(
		"server {\n"
		"  listen 127 8080;\n"
		"  listen 0 8081;\n"
		"  root /var/www/html;\n"
		"  index index.html index.htm;\n"
		"  client_max_body_size 1m;\n"
		"  autoindexing off;\n"
		"  error_page 404 /404.html;\n"
		"  error_page 500 /500.html;\n"
		"\n"
		"  location / {\n"
		"    allowed_methods GET POST;\n"
		"    autoindexing on;\n"
		"  }\n"
		"\n"
		"  location /upload {\n"
		"    allowed_methods POST DELETE;\n"
		"    upload_enable on;\n"
		"    upload_path /var/www/uploads;\n"
		"    client_max_body_size 10m;\n"
		"  }\n"
		"\n"
		"  location /cgi-bin {\n"
		"    cgi_extension .php;\n"
		"    cgi_path /usr/bin/php-cgi;\n"
		"  }\n"
		"}\n"
		"server {\n"
		"  listen 127 9090;\n"
		"  root /var/www/site2;\n"
		"}"
	);

	ASSERT_EQ(_servers.size(), 2);

	EXPECT_EQ(server(0)->_listen.size(), 2);
	EXPECT_EQ(server(0)->_listen[0]._port, 8080);
	EXPECT_EQ(server(0)->_listen[1]._port, 8081);
	EXPECT_EQ(server(0)->_locations.size(), 3);
	EXPECT_EQ(server(0)->_defaults._error.size(), 2);
	EXPECT_EQ(strv(server(0)->_defaults._root), "/var/www/html");

	EXPECT_EQ(server(1)->_listen.size(), 1);
	EXPECT_EQ(server(1)->_listen[0]._port, 9090);
	EXPECT_EQ(server(1)->_locations.size(), 0);
	EXPECT_EQ(strv(server(1)->_defaults._root), "/var/www/site2");
}

TEST_F(ConfParserTest, MissingOpenBrace) {
	ASSERT_THROW(parse("server listen 8080; }"), std::runtime_error);
}

TEST_F(ConfParserTest, MissingCloseBrace) {
	ASSERT_THROW(parse("server { listen 127 8080;"), std::runtime_error);
}

TEST_F(ConfParserTest, MissingSemicolon) {
	ASSERT_THROW(parse("server { listen 127 8080 }"), std::runtime_error);
}

TEST_F(ConfParserTest, UnknownDirective) {
	ASSERT_THROW(parse("server { unknown_directive value; }"), std::runtime_error);
}

TEST_F(ConfParserTest, InvalidPath) {
	ASSERT_THROW(parse("server { root relative/path; }"), std::runtime_error);
}

TEST_F(ConfParserTest, InvalidMethod) {
	ASSERT_THROW(parse("server { location / { allowed_methods INVALID; } }"), std::runtime_error);
}

TEST_F(ConfParserTest, InvalidSize) {
	ASSERT_THROW(parse("server { client_max_body_size 10x; }"), std::runtime_error);
}

TEST_F(ConfParserTest, NegativeNumber) {
	ASSERT_THROW(parse("server { listen 127 -8080; }"), std::runtime_error);
}

TEST_F(ConfParserTest, InvalidOnOff) {
	ASSERT_THROW(parse("server { autoindexing maybe; }"), std::runtime_error);
}

TEST_F(ConfParserTest, EmptyAllowedMethods) {
	ASSERT_THROW(parse("server { location / { allowed_methods; } }"), std::runtime_error);
}

TEST_F(ConfParserTest, UnclosedLocation) {
	ASSERT_THROW(parse("server { location / { root /test; }"), std::runtime_error);
}
