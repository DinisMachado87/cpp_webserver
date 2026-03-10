#include "Engine.hpp"
#include "Token.hpp"
#include "server/Server.hpp"
#include <cerrno>
#include <cstring>
#include <fstream>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>

using std::runtime_error;
using std::ifstream;
using std::stringstream;
using std::string;

// Error handeling
static runtime_error	handleError(const string errMsg) {
	return runtime_error( errMsg + strerror(errno));
}

static string readFile(const char* filepath) {
	ifstream file(filepath);

	if (!file.is_open())
		throw ("Error opening file: ");

	stringstream buffer;
	buffer << file.rdbuf();

	file.close();
	return buffer.str();
}

int	main(int argc, char **argv)
{
	if (argc < 2)
		throw handleError("Missing Argument. Use: ./webserv <config/path>");
	else if (argc > 3)
		throw handleError("Too many arguments. Use: ./webserv <config/path>");

	const char* configPath = argv[1];
	string config = readFile(configPath);
	Engine	engine;
	engine.run(config);

	return (0);
}
