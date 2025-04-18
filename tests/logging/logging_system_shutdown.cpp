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

class LoggingSystemTest : public ::testing::Test {
    protected:
        MockLogSink* mockSink = nullptr;
        void SetUp() override {
            auto uniqueMock = std::make_unique<MockLogSink>();
            mockSink = uniqueMock.get();
    
            std::vector<std::unique_ptr<transity::logging::ILogSink>> testSinks;
            testSinks.push_back(std::move(uniqueMock));
    
            transity::logging::LoggingSystem::getInstance().setSinksForTesting(std::move(testSinks));
        }
        void TearDown() override {
            transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
            logger.activeSinks.clear();
        }
    };

TEST_F(LoggingSystemTest, ShutdownFlushesSinks) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize();
    logger.log(transity::logging::LogLevel::INFO, "Logger", "This is a test message.");

    logger.shutdown();

    ASSERT_TRUE(mockSink->messagesReceived.empty());
}