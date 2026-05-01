#include "Expect.hpp"
#include "Colors.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

using std::cout;
using std::endl;
using std::map;
using std::pair;
using std::runtime_error;
using std::string;
using std::vector;

// Public constructors and destructors
Expect::Expect(Token &token) :
	_token(token) {}

Expect::~Expect() {}

// Error Handler
std::runtime_error Expect::parsingErr(const char *expected) const {
	std::ostringstream oss;
	oss << "Error Parsing config: "
		<< "Expected \"" << expected << "\" "
		<< "got \"" << _token.getString() << "\" "
		<< "in line " << _token.getLineN() << "\"";

	return std::runtime_error(oss.str());
}

// Public Methods
uchar Expect::method() {
	static const uchar size = 4;

	for (int i = 1; i < size; i++)
		if (true == _token.compare(g_methods[i]))
			return (i);
	return Location::DEFAULT;
}

bool Expect::onOff() {
	_token.loadNextOfType(Token::WORD, "\"on/off\"");
	if (_token.compare("on"))
		return (true);
	if (_token.compare("off"))
		return (false);
	throw parsingErr("\"on/off\"");
}

uchar Expect::word(const char *str) {
	_token.loadNextOfType(Token::WORD, str);
	if (!_token.compare(str))
		throw parsingErr(str);
	return Token::WORD;
}

Span<StrView> Expect::wordVec(std::vector<StrView> &vecBuf, uint &vecCursor) {
	uint count = 0;
	while (1) {
		switch (_token.loadNext()) {
		case Token::WORD:
			vecBuf.push_back(_token.getStrV());
			count++;
			break;
		case Token::SEMICOLON: {
			Span<StrView> ret(vecBuf, vecCursor, count);
			vecCursor += count;
			return ret;
		}
		default:
			throw parsingErr("WORD");
		}
	}
}

void Expect::errorPage(map<uint, StrView> &errorMap, string &strBuf) {
	uint code = nextInteger();

	// Empty StrView placeholder
	pair<uint, StrView> placeholderEntry(code, StrView(strBuf));

	// Insert empty StrView placeholder to reserve slot in map
	// and pass a stable pointer rather than a copy to _expect.path();
	// insert() returns pair<iterator, bool> where:
	//   .first  = iterator pointing to the element (inserted or existing)
	//   .second = true if inserted, false if key already existed
	pair<map<uint, StrView>::iterator, bool> insertResult
		= errorMap.insert(placeholderEntry);

	// Get pointer to the StrView now stored in the map
	map<uint, StrView>::iterator errorIter = insertResult.first;
	StrView *pathPtr = &errorIter->second;

	// Fill the empty StrView and register it for consolidation
	path(pathPtr);
}

void Expect::path(StrView *dest) {
	_token.loadNextOfType(Token::WORD, "/<PATH>");
	if (_token.getStrV().getStart()[0] == '/') {
		*dest = _token.getStrV();
		_token.trackInUseToken(dest);
		return;
	}
	throw parsingErr("/<PATH>");
}

void Expect::paths(StrView *paths, int n) {
	for (int i = 0; i < n; i++) {
		path(&paths[i]);
	}
}

uint Expect::findNextDivider(StrView &view) {
	uint newLen = view.getLen();
	if (1 >= newLen)
		return newLen;

	size_t nextDivider = view.find('/', 1);
	newLen = (nextDivider == string::npos) ? view.getLen()
										   : static_cast<uint>(nextDivider);
	return newLen;
}

void Expect::printPathSegs(vector<StrView> &segments, uint i, uint writeIdx,
						   uint deletedSegs) {
	uint len = segments.size() - deletedSegs;

	for (size_t j = 0; j < segments.size(); j++) {
		const bool wi = (j == writeIdx);
		const bool cur = (j == i);
		const char *color = (wi ? RED : cur ? GREEN : "");
		const char *reset = ((wi || cur) ? RESET : "");

		cout << color << ((wi || cur) ? "[" : " ") << reset;
		cout << segments[j];
		cout << color << ((wi || cur) ? "]" : " ") << reset;

		((j + 1) == len ? cout << YELLOW << " | " << RESET : cout << "   ");
	}
	cout << " |len: " << len << endl;
}

void Expect::consolidatePath(vector<StrView> &segments, size_t &writeIdx,
							 bool &hasChanges) {
	DEBUG(uint deletedSegs = 0;);
	for (size_t i = 0; i < segments.size(); i++) {
		DEBUG(std::cout << "\nbefore: ";);
		DEBUG(printPathSegs(segments, i, writeIdx, deletedSegs););

		StrView seg = segments[i];
		bool isLastSegment = (i == (segments.size() - 1));
		if (isLastSegment && seg.getLen() == 2 && seg.ncompare("/.", 2))
			DEBUG(std::cout << "SKIP - not advance writeidx for next loop\n";);
		else if (seg.getLen() == 3 && seg.ncompare("/..", 3)) {
			if (writeIdx <= 0)
				throw runtime_error("Path contains negative level");
			writeIdx--;
			DEBUG(deletedSegs++;);
			DEBUG(std::cout << "GO BACK: " << '\n';);
		} else {
			if (writeIdx != i)
				segments[writeIdx].setStartAndLen(segments[i].getStart(),
												  segments[i].getLen());
			writeIdx++;
			DEBUG(std::cout << "NORMAL path - keep it = copy to write idx  "
							<< '\n';);
		}
		if (writeIdx != i)
			hasChanges = true;
		DEBUG(std::cout << "after:  ";);
		DEBUG(printPathSegs(segments, i, writeIdx, deletedSegs););
	}
}

void Expect::consolidatedPath(StrView *destPath, Request *request) {
	path(destPath);
	if (destPath->getLen() == 2 && destPath->ncompare("/.", 2))
		destPath->setLen(1);
	if (!destPath || destPath->getLen() <= 1)
		return;

	vector<StrView> segments = destPath->splitPath();

	size_t writeIdx = 0;
	bool hasChanges = false;
	consolidatePath(segments, writeIdx, hasChanges);

	DEBUG(cout << *destPath << " | before" << '\n';);
	if (writeIdx == 0)
		destPath->setLen(1);

	if (request) {
		if ('/' == *segments.back().getEnd())
			request->_isDir = true;
		else {
			StrView sufix = segments.back().lastSplitBefore('.');
			if ('.' == *sufix.getStart()) {
				request->_isCgi = true;
				request->_cgiExtension = sufix;
			}
		}
	}

	if (hasChanges) {
		string newPathStr;
		newPathStr.reserve(destPath->getLen());

		for (size_t i = 0; i < writeIdx; i++)
			newPathStr.append(segments[i].getStart(), segments[i].getLen());

		DEBUG(cout << "\nCOPYING:\n" << newPathStr << " | temp str" << '\n';);
		destPath->nreplace(0, StrView(newPathStr), newPathStr.size());
		destPath->setLen(newPathStr.size());
	}
	DEBUG(cout << *destPath << " | after" << endl;);
}

size_t Expect::applySizeUnit(size_t value, char unit) {
	if (unit == '\0')
		return value;

	size_t multiplier;
	switch (tolower(unit)) {
	case 'k':
		multiplier = 1024;
		break;
	case 'm':
		multiplier = 1024 * 1024;
		break;
	case 'g':
		multiplier = 1024 * 1024 * 1024;
		break;
	default:
		throw parsingErr("Invalid size unit (use k, m, or g)");
	}

	if (value > std::numeric_limits<size_t>::max() / multiplier)
		throw parsingErr("Size value too large");
	return value * multiplier;
}

long Expect::number(const char **endPtr) {
	StrView token = _token.getStrV();
	const char *start = token.getStart();
	const char *tokenEnd = start + token.getLen();

	errno = 0;
	char *parseEnd;
	long result = strtol(start, &parseEnd, 10);

	if (errno == ERANGE)
		throw parsingErr("Number out of range");
	if (parseEnd == start)
		throw parsingErr("Expected number");
	if (parseEnd > tokenEnd)
		throw parsingErr("Invalid number format");
	if (result < 0)
		throw parsingErr("Negative number not allowed");

	*endPtr = parseEnd;
	return result;
}

int Expect::integer() {
	const char *end;
	long result = number(&end);

	StrView token = _token.getStrV();
	if (end != token.getStart() + token.getLen())
		throw parsingErr("Unexpected characters after number");
	if (result > INT_MAX)
		throw parsingErr("Number exceeds INT_MAX");

	return static_cast<int>(result);
}

int Expect::nextInteger() {
	_token.loadNextOfType(Token::WORD, "word");
	return integer();
}

size_t Expect::size() {
	_token.loadNextOfType(Token::WORD, "word");

	const char *end;
	long result = number(&end);
	size_t size = static_cast<size_t>(result);
	StrView token = _token.getStrV();
	const char *tokenEnd = token.getStart() + token.getLen();

	if (end != tokenEnd) {
		size = applySizeUnit(size, *end);
		if (++end != tokenEnd)
			throw parsingErr("Invalid characters after size unit");
	}
	return size;
}

in_addr_t Expect::ip(string &ipStr) {
	const size_t nOctets = 4;

	if (ipStr == "*" || ipStr == "0.0.0.0")
		return INADDR_ANY;

	if (ipStr == "localhost")
		ipStr = "127.0.0.1";

	uchar octets[nOctets];
	size_t start = 0;

	for (size_t i = 0; i < nOctets; i++) {
		size_t dotPos = (i < 3) ? ipStr.find('.', start) : ipStr.length();

		if (dotPos == string::npos
			|| dotPos == start) // npos == not found/no position
			throw parsingErr("Invalid IP address");

		string octetStr = ipStr.substr(start, dotPos - start);

		char *end;
		errno = 0;
		long octet = strtol(octetStr.c_str(), &end, 10);

		if (errno == ERANGE || *end != '\0' || octet < 0 || octet > 255)
			throw parsingErr("Invalid IP address");

		octets[i] = static_cast<uchar>(octet);
		start = dotPos + 1;
	}

	uint32_t result = 0;
	result |= octets[0];
	result |= octets[1] << 8;
	result |= octets[2] << 16;
	result |= octets[3] << 24;
	return static_cast<in_addr_t>(result);
}

uint16_t Expect::port(const string &portStr) {
	if (portStr.empty())
		throw parsingErr("Invalid port number");

	char *end;
	errno = 0;
	long port = strtol(portStr.c_str(), &end, 10);

	if (errno == ERANGE || *end != '\0' || port < 1 || port > 65535)
		throw parsingErr("Port must be between 1 and 65535");

	return static_cast<uint16_t>(port);
}
