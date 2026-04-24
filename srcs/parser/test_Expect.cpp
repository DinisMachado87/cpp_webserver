// ExpectTest.cpp
#include "Expect.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

class ExpectTest : public ::testing::Test {
protected:
	std::string _buffer;
	Token *_token;
	Expect *_expect;

	void SetUp() {
		_token = new Token(Token::configDelimiters(), _buffer);
		_expect = new Expect(*_token);
	}

	void TearDown() {
		delete _expect;
		delete _token;
	}

	void testConsolidation(const std::string &input,
						   const std::string &expected) {
		_buffer = input;
		StrView path(_buffer);
		_expect->consolidatedPath(&path);
		std::string result = path.getStr();
		EXPECT_EQ(result, expected);
	}

	void testConsolidationThrows(const std::string &input) {
		_buffer = input;
		StrView path(_buffer);
		EXPECT_THROW(_expect->consolidatedPath(&path), std::runtime_error)
			<< "Input: \"" << input << "\" should throw";
	}
};

TEST_F(ExpectTest, ConsolidatePath_SimplePath) {
	testConsolidation("/a/b/c", "/a/b/c");
}

TEST_F(ExpectTest, ConsolidatePath_RootPath) { testConsolidation("/", "/"); }

TEST_F(ExpectTest, ConsolidatePath_EmptyPath) {
	_buffer = "";
	StrView path(_buffer);
	EXPECT_THROW(_expect->consolidatedPath(&path), std::runtime_error);
}

// Handling "." (current directory)
TEST_F(ExpectTest, ConsolidatePath_CurrentDirectory) {
	testConsolidation("/a/./b", "/a/b");
}

TEST_F(ExpectTest, ConsolidatePath_MultipleDots) {
	testConsolidation("/a/././b/./c", "/a/b/c");
}

TEST_F(ExpectTest, ConsolidatePath_OnlyDot) { testConsolidation("/.", "/"); }

// Handling ".." (parent directory)
TEST_F(ExpectTest, ConsolidatePath_ParentDirectory) {
	testConsolidation("/a/b/../c", "/a/c");
}

TEST_F(ExpectTest, ConsolidatePath_MultipleParents) {
	testConsolidation("/a/b/c/../../d", "/a/d");
}

TEST_F(ExpectTest, ConsolidatePath_ParentAtEnd) {
	testConsolidation("/a/b/..", "/a");
}

TEST_F(ExpectTest, ConsolidatePath_ComplexPath) {
	testConsolidation("/a/b/../c/./d/../e", "/a/c/e");
}

TEST_F(ExpectTest, ConsolidatePath_AllTheWayBack) {
	testConsolidation("/a/b/../../", "/");
}

// Edge cases with consecutive slashes
TEST_F(ExpectTest, ConsolidatePath_DoubleSlash) {
	testConsolidation("/a//b", "/a/b");
}

TEST_F(ExpectTest, ConsolidatePath_TripleSlash) {
	testConsolidation("/a///b///c", "/a/b/c");
}

TEST_F(ExpectTest, ConsolidatePath_LeadingDoubleSlash) {
	testConsolidation("//a/b", "/a/b");
}

// Mixing . and ..
TEST_F(ExpectTest, ConsolidatePath_MixedDotsDotDots) {
	testConsolidation("/a/./b/../c/./d", "/a/c/d");
}

TEST_F(ExpectTest, ConsolidatePath_DotBeforeDotDot) {
	testConsolidation("/a/./b/./../c", "/a/c");
}

// Error cases - negative levels (going above root)
TEST_F(ExpectTest, ConsolidatePath_NegativeLevel_SingleDotDot) {
	testConsolidationThrows("/..");
}

TEST_F(ExpectTest, ConsolidatePath_NegativeLevel_MultipleDotDot) {
	testConsolidationThrows("/../..");
}

TEST_F(ExpectTest, ConsolidatePath_NegativeLevel_AfterPath) {
	testConsolidationThrows("/a/../..");
}

TEST_F(ExpectTest, ConsolidatePath_NegativeLevel_Complex) {
	testConsolidationThrows("/a/b/../../..");
}

// Real-world HTTP paths
TEST_F(ExpectTest, ConsolidatePath_TypicalWebPath) {
	testConsolidation("/var/www/html/index.html", "/var/www/html/index.html");
}

TEST_F(ExpectTest, ConsolidatePath_RelativeWebPath) {
	testConsolidation("/images/../css/style.css", "/css/style.css");
}

TEST_F(ExpectTest, ConsolidatePath_NestedResources) {
	testConsolidation("/api/v1/../../public/docs", "/public/docs");
}

// Potential attack vectors (path traversal attempts)
TEST_F(ExpectTest, ConsolidatePath_PathTraversalAttempt) {
	testConsolidationThrows("/public/../../etc/passwd");
}

TEST_F(ExpectTest, ConsolidatePath_MultipleTraversalAttempt) {
	testConsolidationThrows("/public/../../../root");
}

// Long paths
TEST_F(ExpectTest, ConsolidatePath_DeepPath) {
	testConsolidation("/a/b/c/d/e/f/g/h/i/j", "/a/b/c/d/e/f/g/h/i/j");
}

TEST_F(ExpectTest, ConsolidatePath_DeepWithDotDots) {
	testConsolidation("/a/b/c/d/../e/../f/g", "/a/b/c/f/g");
}

// Trailing slashes (if your implementation should handle them)
TEST_F(ExpectTest, ConsolidatePath_TrailingSlash) {
	testConsolidation("/a/b/c/", "/a/b/c/");
}

TEST_F(ExpectTest, ConsolidatePath_TrailingSlashWithDotDot) {
	testConsolidation("/a/b/../", "/a/");
}

// Edge case: segments that look like .. but aren't
TEST_F(ExpectTest, ConsolidatePath_DotDotInFilename) {
	testConsolidation("/a/..b/c", "/a/..b/c");
}

TEST_F(ExpectTest, ConsolidatePath_DotDotDot) {
	testConsolidation("/a/.../b", "/a/.../b");
}
