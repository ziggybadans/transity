/**
 * @file logging_system_shutdown.cpp
 * @brief Tests for logging system shutdown behavior
 *
 * Verifies that shutdown:
 * - Properly flushes all sinks
 * - Cleans up resources
 */
#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <filesystem>

#include "logging/LoggingSystem.hpp"
#include "logging/ILogSink.h"

/**
 * @class MockLogSink
 * @brief Mock implementation of ILogSink for testing shutdown
 *
 * Captures messages and clears them on flush to verify
 * shutdown behavior
 */
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

/**
 * @class LoggingSystemTest
 * @brief Test fixture for logging system shutdown tests
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
 * @test ShutdownFlushesSinks
 * @brief Verifies shutdown flushes all sinks
 *
 * Tests that calling shutdown:
 * - Triggers flush on all sinks
 * - Clears any pending messages
 */
TEST_F(LoggingSystemTest, ShutdownFlushesSinks) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    logger.initialize();
    logger.log(transity::logging::LogLevel::INFO, "Logger", "This is a test message.");

    logger.shutdown();

    ASSERT_TRUE(mockSink->messagesReceived.empty());
}

TEST(LoggingSystemShutdown, FileSinkClosesOnFile) {
    transity::logging::LoggingSystem& logger = transity::logging::LoggingSystem::getInstance();
    const std::string tempLogDir = "./temp_closure_test_logs"; // Directory path
    const std::string testMessage = "== FileSinkClosureTest message ==";

    // Cleanup before starting
    std::filesystem::remove_all(tempLogDir);

    // Initialize logger to write to the directory
    logger.initialize(transity::logging::LogLevel::INFO, true, false, tempLogDir);

    // Log the message
    logger.log(transity::logging::LogLevel::INFO, "ClosureTest", testMessage.c_str());

    // Shutdown
    logger.shutdown();

    // --- Find the actual log file ---
    std::string actualLogFilePath = "";
    try {
        for (const auto& entry : std::filesystem::directory_iterator(tempLogDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                actualLogFilePath = entry.path().string();
                break; // Found it
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        FAIL() << "Filesystem error finding log file: " << e.what();
    }

    ASSERT_FALSE(actualLogFilePath.empty()) << "Could not find .log file in " << tempLogDir;

    // --- Read back the actual log file ---
    std::ifstream logFile(actualLogFilePath);
    ASSERT_TRUE(logFile.is_open()) << "Failed to open actual log file: " << actualLogFilePath;

    std::stringstream buffer;
    buffer << logFile.rdbuf();
    std::string fileContent = buffer.str();
    logFile.close(); // Close it now we're done reading

    // Assert content contains the message
    ASSERT_NE(fileContent.find(testMessage), std::string::npos)
        << "Test message '" << testMessage << "' not found in log file content:\n" << fileContent;

    // Cleanup directory and its contents
    std::filesystem::remove_all(tempLogDir);
}