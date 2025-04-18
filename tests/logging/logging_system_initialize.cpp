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
    void flush() override {}
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

TEST(LoggingSystem, InitializesWithDefaultConfig) {
    transity::logging::LoggingSystem logger;

    logger.initialize();

    ASSERT_EQ(logger.getLogLevel(), transity::logging::LogLevel::INFO);
    ASSERT_TRUE(logger.isConsoleSinkEnabled());
    ASSERT_TRUE(logger.isFileSinkEnabled());
}

TEST(LoggingSystem, InitializesWithCustomConfig) {
    transity::logging::LoggingSystem logger;
    transity::logging::LogLevel customLevel = transity::logging::LogLevel::DEBUG;
    bool enableFileSink = true;
    std::string filePath = "custom_log.txt";
    bool enableConsoleSink = false;

    logger.initialize(customLevel, enableFileSink, enableConsoleSink, filePath);

    ASSERT_EQ(logger.getLogLevel(), customLevel);
    ASSERT_EQ(logger.isFileSinkEnabled(), enableFileSink);
    ASSERT_EQ(logger.isConsoleSinkEnabled(), enableConsoleSink);
    ASSERT_EQ(logger.getFilePath(), filePath);
}

TEST(LoggingSystem, ConsoleSinkInitializes) {
    transity::logging::LoggingSystem logger;

    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    ASSERT_TRUE(logger.isConsoleSinkEnabled());
}

TEST(LoggingSystem, FileSinkInitializes) {
    transity::logging::LoggingSystem logger;

    logger.initialize(transity::logging::LogLevel::INFO, true, false);

    ASSERT_TRUE(logger.isFileSinkEnabled());
}

TEST(LoggingSystem, FileSinkHandlesErrors) {
    transity::logging::LoggingSystem logger;

    ASSERT_THROW(
        logger.initialize(transity::logging::LogLevel::INFO, true, false, "invalid/path.txt"),
        std::runtime_error
    );
}

TEST(LoggingSystem, InitializationMessageLogged) {
    TestableLoggingSystem logger;
    std::string expectedMessage = "Logging system started. Level: INFO. Sinks: Console, File.";

    logger.initialize();

    ASSERT_EQ(logger.mockSink->messagesReceived.size(), 1);
    ASSERT_EQ(logger.mockSink->messagesReceived[0], expectedMessage);
}