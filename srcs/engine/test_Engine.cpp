#include "Engine.hpp"
#include "Server.hpp"
#include <gtest/gtest.h>

class TestableEngine: public Engine {
public:
	int	getFdEpoll() { return _fdEpoll; };

};

class EngineTest: public ::testing::Test {
protected:
};

TEST_F(EngineTest, CreateSuccessfully) {
	TestableEngine engine;
	EXPECT_TRUE(engine.getFdEpoll());
	// engine.addSocket(Listening());
};

