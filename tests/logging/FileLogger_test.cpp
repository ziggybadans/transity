#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logging/FileLogger.h"

#include <filesystem>
#include <fstream>

class FileLoggerTest : public ::testing::Test {
protected:
    const std::string testLogFilename = "test.log";

    void SetUp() override {
        std::filesystem::remove(testLogFilename);
    }

    void TearDown() override {
        std::filesystem::remove(testLogFilename);
    }
};

TEST_F(FileLoggerTest, CreatesFileOnConstruction) {
    Transity::Logging::FileLogger logger(testLogFilename);
    EXPECT_TRUE(std::filesystem::exists(testLogFilename));
}

TEST_F(FileLoggerTest, WritesMessageToFile) {
    Transity::Logging::FileLogger logger(testLogFilename);
    const std::string message = "This is a test message";
    logger.log(Transity::Logging::LogLevel::INFO, message);
    std::ifstream file(testLogFilename);
    ASSERT_TRUE(file.is_open());

    std::string fileContent;
    std::getline(file, fileContent);
    file.close();

    EXPECT_THAT(fileContent, testing::HasSubstr(message));
}

TEST_F(FileLoggerTest, FormatsMessage) {
    Transity::Logging::FileLogger logger(testLogFilename);
    const std::string message = "This is a formatted test message";
    const Transity::Logging::LogLevel level = Transity::Logging::LogLevel::WARN;
    const std::string expectedLevelStr = Transity::Logging::logLevelToString(level);

    logger.log(level, message);
    std::ifstream file(testLogFilename);
    ASSERT_TRUE(file.is_open());
    std::string fileContent;
    std::getline(file, fileContent);
    file.close();

    EXPECT_THAT(fileContent, testing::HasSubstr("[" + expectedLevelStr + "]"));
    EXPECT_THAT(fileContent, testing::HasSubstr(message));
    std::string expectedPatternStr = "\\[\\d\\d\\d\\d-\\d\\d-\\d\\d \\d\\d:\\d\\d:\\d\\d\\]";
    EXPECT_THAT(fileContent, testing::ContainsRegex(expectedPatternStr));
}