#include "Location.hpp"
#include "Overrides.hpp"
#include "Span.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <bitset>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <sstream>

using std::bitset;
using std::ostream;
using std::size_t;
using std::stringstream;

const char *Location::_methodStrs[4] = {"DEFAULT", "GET", "POST", "DELETE"};

// Public constructors and destructors

Location::Location(std::vector<StrView> &vecBuf) :
	_overrides(vecBuf),
	_cgiExtensions(vecBuf),
	_cgiPath(vecBuf),
	_returnCode(0),
	_uploadEnable(false),
	_allowedMethods(DEFAULT) {}

Location &Location::operator=(const Location &other) {
	if (this == &other) {
		return *this;
	}
	_overrides = other._overrides;
	_cgiExtensions = other._cgiExtensions;
	_cgiPath = other._cgiPath;
	_path = other._path;
	_returnPath = other._returnPath;
	_rewrite_old = other._rewrite_old;
	_rewrite_new = other._rewrite_new;
	_uploadPath = other._uploadPath;
	_returnCode = other._returnCode;
	_uploadEnable = other._uploadEnable;
	_allowedMethods = other._allowedMethods;
	return *this;
}

const Overrides &Location::getOverrides() const { return _overrides; }
const char *Location::getPath() const { return _path.getStart(); }
const char *Location::getReturnPath() const { return _returnPath.getStart(); }
const char *Location::getUploadPath() const { return _uploadPath.getStart(); }
uint Location::getReturncode() const { return _returnCode; }
bool Location::getUploadEnabled() const { return _uploadEnable; }

const char *Location::getRewriteOldPath() const {
	return _rewrite_old.getStart();
}

const char *Location::getRewriteNewPath() const {
	return _rewrite_new.getStart();
}

uchar Location::isAllowedMethod(uchar methodToCheck) const {
	return _allowedMethods & (1 << methodToCheck);
};

// CGI
const Span<StrView> &Location::getCgiPath() const { return _cgiPath; }

const Span<StrView> &Location::getCgiExtensions() const {
	return _cgiExtensions;
}

const char *Location::findCgiPath(StrView &extention) const {
	return findCgiPath(extention.getStart());
}

const char *Location::findCgiPath(const char *extention) const {
	for (size_t i = 0; i < _cgiExtensions.len(); i++)
		if (_cgiExtensions[i].compare(extention))
			return _cgiPath[i].getStart();
	return NULL;
}

const char *Location::safeStr(const char *str) const {
	return str ? str : "NULL";
}

void Location::printStrvSpan(const char *msg, const Span<StrView> &span,
							 ostream &stream) const {
	size_t i = 0;
	stream << msg;
	for (i = 0; i < span.len(); i++)
		stream << safeStr(span[i].getStart()) << ", ";
	if (i == 0)
		stream << "NONE";
	stream << '\n';
}

void Location::printMethods(ostream &stream) const {
	bool none = true;
	stream << "\tAllowed Methods (bitset: " << bitset<8>(_allowedMethods)
		   << "): ";
	for (size_t method = GET; method <= DELETE; method++)
		if ((1 << method) & _allowedMethods) {
			stream << _methodStrs[method] << " ,";
			none = false;
		}
	if (none)
		stream << "NONE";
	stream << '\n';
}

void Location::printLocation(ssize_t index, ostream &stream) const {
	if (DEFAULT_LOCATION == index)
		stream << "Deafault Location:\n";
	else
		stream << "  [" << index << "] Path: " << safeStr(getPath()) << '\n';

	stream << "\tReturn Code: " << getReturncode() << '\n';
	stream << "\tReturn Path: " << getReturnPath() << '\n';
	stream << "\tUpload Enabled: " << (getUploadEnabled() ? "true" : "false")
		   << '\n';
	stream << "\tUpload Path: " << safeStr(getUploadPath()) << '\n';

	printStrvSpan("\tCGI Extensions: ", _cgiExtensions, stream);
	printStrvSpan("\tCGI Paths: ", _cgiPath, stream);

	printMethods(stream);

	_overrides.printOverrides("\tOverrrides", stream);
}
