#include <gtest/gtest.h>
#include <regex>

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

    logger.log("This is a TRACE message", transity::logging::LogLevel::TRACE);
    logger.log("This is a DEBUG message", transity::logging::LogLevel::DEBUG);
    logger.log("This is an INFO message", transity::logging::LogLevel::INFO);

    ASSERT_EQ(logger.mockSink->messagesReceived.size(), 2);
}

TEST(LoggingSystem, FormatsMessage) {
    TestableLoggingSystem logger;
    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    logger.log("This is a formatted message.", transity::logging::LogLevel::INFO);

    std::cout << "\n--- Log Output (FormatsMessage Test) ---\n";
    if (!logger.mockSink->messagesReceived.empty()) {
         // Print the last message received by the mock
         std::cout << logger.mockSink->messagesReceived.back() << std::endl;
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