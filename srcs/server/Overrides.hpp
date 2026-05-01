#ifndef OVERRIDES_HPP
#define OVERRIDES_HPP

#include "Span.hpp"
#include "StrView.hpp"
#include <cstddef>
#include <map>
#include <ostream>
#include <string>

struct Overrides {
	// Constructor
	Overrides(std::string &buffer, std::vector<StrView> &vecBuf);
	// Vars
	std::map<uint, StrView> _error;
	Span<StrView> _index;
	StrView _root;
	size_t _clientMaxBody;
	size_t _uploadMaxBody;
	bool _autoindex;
	// Getters
	size_t getClientMaxBody() const;
	size_t getUploadMaxBody() const;
	const char *findErrorFile(uint errorCode) const;
	bool isAutoindexed() const;
	const char *getRoot() const;
	const Span<StrView> &getIndex() const;
	size_t getErrorMapSize() const;
	const char *safeStr(const char *str) const;
	void printOverrides(const char *label, std::ostream &stream) const;
	void printMap(const char *label, std::ostream &stream) const;
};

#endif
