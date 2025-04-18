#include <gtest/gtest.h>
#include <vector>
#include <memory>

#include "logging/logging_system.h"
#include "logging/ILogSink.h"

class MockLogSink : public transity::logging::ILogSink {
public:
    void write(const std::string& message) override {
        messagesReceived.push_back(message);
    }
    void flush() override {
        messagesReceived.clear();
    }
    std::vector<std::string> messagesReceived;
};

class TestableLoggingSystem : public transity::logging::LoggingSystem {
public:
    MockLogSink* mockSink = nullptr;
    void initializeSinks() override {
        activeSinks.clear();
        auto uniqueMock = std::make_unique<MockLogSink>();
        mockSink = uniqueMock.get();
        activeSinks.push_back(std::move(uniqueMock));
    }
};

TEST(LoggingSystem, ShutdownFlushesSinks) {
    TestableLoggingSystem logger;
    logger.initialize();
    logger.log(transity::logging::LogLevel::INFO, "Logger", "This is a test message.");

    logger.shutdown();

    ASSERT_TRUE(logger.mockSink->messagesReceived.empty());
}