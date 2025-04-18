/**
 * @file logging_system_initialize.cpp
 * @brief Tests for logging system initialization
 *
 * Verifies:
 * - Default initialization configuration
 * - Custom initialization parameters
 * - Sink initialization behavior
 * - Error handling during initialization
 * - Initialization message logging
 */
#include <gtest/gtest.h>
#include <vector>
#include <memory>

#include "logging/logging_system.h"
#include "logging/ILogSink.h"

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
 * @brief Test fixture for logging system initialization tests
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
    }
};

/**
 * @test InitializesWithDefaultConfig
 * @brief Verifies default initialization values
 *
 * Tests that default initialization sets:
 * - Log level to INFO
 * - Both console and file sinks enabled
 */
TEST_F(LoggingSystemTest, InitializesWithDefaultConfig) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize();

    ASSERT_EQ(logger.getLogLevel(), transity::logging::LogLevel::INFO);
    ASSERT_TRUE(logger.isConsoleSinkEnabled());
    ASSERT_TRUE(logger.isFileSinkEnabled());
}

/**
 * @test InitializesWithCustomConfig
 * @brief Verifies custom initialization parameters
 *
 * Tests that initialization with custom parameters:
 * - Correctly sets log level
 * - Properly enables/disables sinks
 * - Sets custom file path
 */
TEST_F(LoggingSystemTest, InitializesWithCustomConfig) {
    transity::logging::LogLevel customLevel = transity::logging::LogLevel::DEBUG;
    bool enableFileSink = true;
    std::string filePath = "custom_log.txt";
    bool enableConsoleSink = false;

    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize(customLevel, enableFileSink, enableConsoleSink, filePath);

    ASSERT_EQ(logger.getLogLevel(), customLevel);
    ASSERT_EQ(logger.isFileSinkEnabled(), enableFileSink);
    ASSERT_EQ(logger.isConsoleSinkEnabled(), enableConsoleSink);
    ASSERT_EQ(logger.getFilePath(), filePath);
}

/**
 * @test ConsoleSinkInitializes
 * @brief Verifies console sink initialization
 *
 * Tests that console sink is properly enabled
 * when configured during initialization
 */
TEST_F(LoggingSystemTest, ConsoleSinkInitializes) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();

    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    ASSERT_TRUE(logger.isConsoleSinkEnabled());
}

/**
 * @test FileSinkInitializes
 * @brief Verifies file sink initialization
 *
 * Tests that file sink is properly enabled
 * when configured during initialization
 */
TEST_F(LoggingSystemTest, FileSinkInitializes) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();

    logger.initialize(transity::logging::LogLevel::INFO, true, false);

    ASSERT_TRUE(logger.isFileSinkEnabled());
}

/**
 * @test FileSinkHandlesErrors
 * @brief Verifies error handling during file sink initialization
 *
 * Tests that invalid file paths throw appropriate exceptions
 * during initialization
 */
TEST(LoggingSystem, FileSinkHandlesErrors) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();

    ASSERT_THROW(
        logger.initialize(transity::logging::LogLevel::INFO, true, false, "invalid/path.txt"),
        std::runtime_error
    );
}

/**
 * @test InitializationMessageLogged
 * @brief Verifies initialization message is logged
 *
 * Tests that the system logs its initialization
 * configuration when starting up
 */
TEST_F(LoggingSystemTest, InitializationMessageLogged) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    std::string expectedMessage = "Logging system started. Level: INFO. Sinks: Console, File.";

    logger.initialize();

    std::cout << "\n--- Log Output ---\n";
    if (!mockSink->messagesReceived.empty()) {
         for(const auto& msg : mockSink->messagesReceived) {
             std::cout << msg << std::endl;
         }
    } else {
         std::cout << "(No message received by mock sink)\n";
    }
    std::cout << "---------------------------------------\n";

    ASSERT_NE(mockSink, nullptr);
    ASSERT_EQ(mockSink->messagesReceived.size(), 1);
    ASSERT_EQ(mockSink->messagesReceived[0], expectedMessage);
}