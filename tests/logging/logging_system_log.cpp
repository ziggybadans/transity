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

#include "logging/LoggingSystem.hpp"

/**
 * @class MockLogSink
 * @brief Mock implementation of ILogSink for testing
 *
 * Captures all received messages in a vector for verification
 */
class MockLogSink : public transity::logging::ILogSink {
    public:
        void write(const std::string& message) override {
            std::lock_guard<std::mutex> lock(sinkMutex);
            messagesReceived.push_back(message);
        }
        void flush() override {}
        std::vector<std::string> messagesReceived;
    private:
        std::mutex sinkMutex;
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

TEST_F (LoggingSystemTest, LogsConcurrently) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize(transity::logging::LogLevel::TRACE, false, true);

    const int numThreads = 5;
    const int messagesPerThread = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                logger.log(transity::logging::LogLevel::INFO, "ConcurrentTest", "Thread %d logging message %d", i, j);
                std::this_thread::yield();
            }
        });
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    const size_t expectedTotalMessages = (numThreads * messagesPerThread) + 1;
    ASSERT_EQ(mockSink->messagesReceived.size(), expectedTotalMessages);
}

TEST_F(LoggingSystemTest, DispatchesToMultipleSinks) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();

    auto mockSink1_ptr = std::make_unique<MockLogSink>();
    MockLogSink* mockSink1 = mockSink1_ptr.get();

    auto mockSink2_ptr = std::make_unique<MockLogSink>();
    MockLogSink* mockSink2 = mockSink2_ptr.get();

    std::vector<std::unique_ptr<transity::logging::ILogSink>> testSinks;
    testSinks.push_back(std::move(mockSink1_ptr));
    testSinks.push_back(std::move(mockSink2_ptr));

    logger.setSinksForTesting(std::move(testSinks));

    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    logger.log(transity::logging::LogLevel::INFO, "Logger", "This is a test message.");

    ASSERT_EQ(mockSink1->messagesReceived.size(), 2);
    ASSERT_EQ(mockSink2->messagesReceived.size(), 2);
    ASSERT_EQ(mockSink1->messagesReceived, mockSink2->messagesReceived);
}

TEST_F(LoggingSystemTest, HelperMacrosLogCorrectly) {
    // Arrange
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    // Initialize with a low level to capture all test messages
    logger.initialize(transity::logging::LogLevel::TRACE, false, true); // Logs init message

    const std::string debugMsg = "This is a debug macro message.";
    const std::string infoMsg = "This is an info macro message with arg: 42";
    const std::string warnMsg = "This is a warning macro message.";

    // Act: Call various helper macros
    LOG_DEBUG("MacroTest", debugMsg.c_str());
    LOG_INFO("MacroTest", "This is an info macro message with arg: %d", 42);
    LOG_WARN("MacroTest", warnMsg.c_str());
    // Maybe call one that should be filtered out if you change init level
    // LOG_TRACE("MacroTest", "This trace message should be logged.");

    // Assert
    // Expecting: Init message + DEBUG + INFO + WARN = 4 messages
    ASSERT_EQ(mockSink->messagesReceived.size(), 4);

    // Check content of each message (adjust indices based on actual output order if needed)
    // Note: Index 0 is the init message from logger.initialize()

    // Check Debug message (likely index 1)
    ASSERT_NE(mockSink->messagesReceived[1].find("[DEBUG]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[1].find("[MacroTest]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[1].find(debugMsg), std::string::npos);

    // Check Info message (likely index 2)
    ASSERT_NE(mockSink->messagesReceived[2].find("[INFO]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[2].find("[MacroTest]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[2].find(infoMsg), std::string::npos);

    // Check Warn message (likely index 3)
    ASSERT_NE(mockSink->messagesReceived[3].find("[WARN]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[3].find("[MacroTest]"), std::string::npos);
    ASSERT_NE(mockSink->messagesReceived[3].find(warnMsg), std::string::npos);
}