#include <gtest/gtest.h>
#include <regex>
#include <thread>

#include "logging/logging_system.h"

class MockLogSink : public transity::logging::ILogSink {
    public:
        void write(const std::string& message) override {
            messagesReceived.push_back(message);
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

TEST(LoggingSystem, FiltersLevel) {
    TestableLoggingSystem logger;
    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    logger.log(transity::logging::LogLevel::TRACE, "This is a TRACE message");
    logger.log(transity::logging::LogLevel::DEBUG, "This is a DEBUG message");
    logger.log(transity::logging::LogLevel::INFO, "This is an INFO message");

    std::cout << "\n--- Log Output ---\n";
    if (!logger.mockSink->messagesReceived.empty()) {
         for(const auto& msg : logger.mockSink->messagesReceived) {
             std::cout << msg << std::endl;
         }
    } else {
         std::cout << "(No message received by mock sink)\n";
    }
    std::cout << "---------------------------------------\n";

    ASSERT_EQ(logger.mockSink->messagesReceived.size(), 2);
}

TEST(LoggingSystem, FormatsMessage) {
    TestableLoggingSystem logger;
    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    logger.log(transity::logging::LogLevel::INFO, "This is a formatted message.");

    std::cout << "\n--- Log Output ---\n";
    if (!logger.mockSink->messagesReceived.empty()) {
         for(const auto& msg : logger.mockSink->messagesReceived) {
             std::cout << msg << std::endl;
         }
    } else {
         std::cout << "(No message received by mock sink)\n";
    }
    std::cout << "---------------------------------------\n";

    std::regex timestamp_regex(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3} )"); // Note the ^
    ASSERT_EQ(logger.mockSink->messagesReceived.size(), 2);
    ASSERT_TRUE(std::regex_search(logger.mockSink->messagesReceived[1], timestamp_regex));
    ASSERT_NE(logger.mockSink->messagesReceived[1].find("[INFO]"), std::string::npos);
    ASSERT_NE(logger.mockSink->messagesReceived[1].find("This is a formatted message."), std::string::npos);
}

TEST(LoggingSystem, FormatsMessageWithArgs) {
    TestableLoggingSystem logger;
    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    std::cout << "\n--- Log Output ---\n";
    if (!logger.mockSink->messagesReceived.empty()) {
         for(const auto& msg : logger.mockSink->messagesReceived) {
             std::cout << msg << std::endl;
         }
    } else {
         std::cout << "(No message received by mock sink)\n";
    }
    std::cout << "---------------------------------------\n";

    logger.log(transity::logging::LogLevel::INFO, "User %s logged in with ID %d", "TestUser", 123);
    ASSERT_NE(logger.mockSink->messagesReceived[1].find("User TestUser logged in with ID 123"), std::string::npos);
}