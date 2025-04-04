#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logging/FileLogger.h"

#include <filesystem>

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