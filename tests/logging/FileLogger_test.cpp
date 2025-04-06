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

    EXPECT_EQ(fileContent, message);
}