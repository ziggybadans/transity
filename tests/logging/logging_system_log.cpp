/**
 * @file logging_system_log.cpp
 * @brief Tests for logging system message formatting and filtering
 *
 * Verifies:
 * - Log level filtering works correctly
 * - Messages are properly formatted with timestamps, thread IDs, etc.
 * - Variable argument formatting works as expected
 */
#include <gtest/gtest.h>
#include <regex>
#include <thread>

#include "logging/logging_system.h"

/**
 * @class MockLogSink
 * @brief Mock implementation of ILogSink for testing
 *
 * Captures all received messages in a vector for verification
 */
class MockLogSink : public transity::logging::ILogSink {
    public:
        void write(const std::string& message) override {
            messagesReceived.push_back(message);
        }
        void flush() override {}
        std::vector<std::string> messagesReceived;
};

/**
 * @class LoggingSystemTest
 * @brief Test fixture for logging system tests
 *
 * Sets up a mock log sink before each test and cleans up after
 */
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

/**
 * @test FiltersLevel
 * @brief Verifies log level filtering works correctly
 *
 * Tests that:
 * - Messages below current log level are filtered out
 * - Messages at or above current level are passed through
 */
TEST_F(LoggingSystemTest, FiltersLevel) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    logger.log(transity::logging::LogLevel::TRACE, "Logger", "This is a TRACE message");
    logger.log(transity::logging::LogLevel::DEBUG, "Logger", "This is a DEBUG message");
    logger.log(transity::logging::LogLevel::INFO, "Logger", "This is an INFO message");

    std::cout << "\n--- Log Output ---\n";
    if (!mockSink->messagesReceived.empty()) {
         for(const auto& msg : mockSink->messagesReceived) {
             std::cout << msg << std::endl;
         }
    } else {
         std::cout << "(No message received by mock sink)\n";
    }
    std::cout << "---------------------------------------\n";

    ASSERT_EQ(mockSink->messagesReceived.size(), 2);
}

/**
 * @test FormatsMessage
 * @brief Verifies message formatting is correct
 *
 * Tests that messages contain:
 * - Proper timestamp format
 * - Thread ID
 * - Log level
 * - System/component name
 * - Message content
 */
TEST_F(LoggingSystemTest, FormatsMessage) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    logger.log(transity::logging::LogLevel::INFO, "Logger", "This is a formatted message.");

    std::cout << "\n--- Log Output ---\n";
    if (!mockSink->messagesReceived.empty()) {
         for(const auto& msg : mockSink->messagesReceived) {
             std::cout << msg << std::endl;
         }
    } else {
         std::cout << "(No message received by mock sink)\n";
    }
    std::cout << "---------------------------------------\n";

    ASSERT_EQ(mockSink->messagesReceived.size(), 2);
    std::regex timestamp_regex(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3} )"); // Note the ^
    ASSERT_TRUE(std::regex_search(mockSink->messagesReceived[1], timestamp_regex));
    std::regex thread_id_regex(R"(\[TID: \d+\])"); // Example regex
    ASSERT_TRUE(std::regex_search(mockSink->messagesReceived[1], thread_id_regex));
    ASSERT_NE(mockSink->messagesReceived[1].find("[INFO]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[1].find("[Logger]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[1].find("This is a formatted message."), std::string::npos);
}

/**
 * @test FormatsMessageWithArgs
 * @brief Verifies variable argument formatting works
 *
 * Tests that printf-style format strings with arguments
 * are properly expanded in the final log message
 */
TEST_F(LoggingSystemTest, FormatsMessageWithArgs) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    logger.log(transity::logging::LogLevel::INFO, "Logger", "User %s logged in with ID %d", "TestUser", 123);

    std::cout << "\n--- Log Output ---\n";
    if (!mockSink->messagesReceived.empty()) {
         for(const auto& msg : mockSink->messagesReceived) {
             std::cout << msg << std::endl;
         }
    } else {
         std::cout << "(No message received by mock sink)\n";
    }
    std::cout << "---------------------------------------\n";

    ASSERT_NE(mockSink->messagesReceived[1].find("User TestUser logged in with ID 123"), std::string::npos);
}