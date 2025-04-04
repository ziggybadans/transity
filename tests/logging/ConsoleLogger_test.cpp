#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "logging/ConsoleLogger.h"
#include "logging/ILogger.h"
#include "logging/LogUtils.h"

#include <string>
#include <tuple>

class ConsoleLoggerFormatParamTest : public ::testing::TestWithParam<Transity::Logging::LogLevel> {
protected:

};

TEST(ConsoleLoggerTest, WritesMessageToStdOut) {
    Transity::Logging::ConsoleLogger logger(Transity::Logging::LogLevel::INFO);
    
    std::string testMsg = "This is a test message";

    testing::internal::CaptureStdout();
    logger.log(Transity::Logging::LogLevel::INFO, testMsg);
    std::string output = testing::internal::GetCapturedStdout();
    std::cout << output;

    EXPECT_THAT(output, testing::HasSubstr(testMsg));
};

TEST_P(ConsoleLoggerFormatParamTest, FormatMessageCorrectlyForLevel) {
    Transity::Logging::LogLevel currentLevel = GetParam();
    std::string testMsg = "Testing level formatting";
    Transity::Logging::ConsoleLogger logger(Transity::Logging::LogLevel::TRACE);

    std::string output;

    if (currentLevel >= Transity::Logging::LogLevel::ERROR) {
        // Capture stderr for ERROR and FATAL
        testing::internal::CaptureStderr();
        logger.log(currentLevel, testMsg);
        output = testing::internal::GetCapturedStderr();
    } else {
        // Capture stdout for other levels
        testing::internal::CaptureStdout();
        logger.log(currentLevel, testMsg);
        output = testing::internal::GetCapturedStdout();
    }

    std::string expectedLevelStr = logLevelToString(currentLevel);
    std::string expectedPatternStr = "\\[\\d\\d\\d\\d-\\d\\d-\\d\\d \\d\\d:\\d\\d:\\d\\d\\] \\[" +
                                     expectedLevelStr + "\\] " + testMsg + "\\n";

    EXPECT_THAT(output, testing::MatchesRegex(expectedPatternStr));
};

// Instantiate the test suite with all possible LogLevel values
INSTANTIATE_TEST_SUITE_P(
    AllLogLevels, // An identifier for this instantiation
    ConsoleLoggerFormatParamTest, // The test fixture class
    ::testing::Values( // The values to pass as parameters
        Transity::Logging::LogLevel::TRACE,
        Transity::Logging::LogLevel::DEBUG,
        Transity::Logging::LogLevel::INFO,
        Transity::Logging::LogLevel::WARN,
        Transity::Logging::LogLevel::ERROR,
        Transity::Logging::LogLevel::FATAL
    )
    // Optional: A function/lambda to generate meaningful test names
    //[](const testing::TestParamInfo<Transity::Logging::LogLevel>& info) {
    //    return logLevelToString(info.param);
    //}
);

TEST(ConsoleLoggerTest, FilterMessagesBelowMinLevel) {
    Transity::Logging::ConsoleLogger logger(Transity::Logging::LogLevel::INFO);

    std::string debugMsg = "This is a debug message.";
    std::string infoMsg = "This is an info message.";
    std::string warnMsg = "This is a warning message.";

    testing::internal::CaptureStdout();
    logger.log(Transity::Logging::LogLevel::DEBUG, debugMsg);
    logger.log(Transity::Logging::LogLevel::INFO, infoMsg);
    logger.log(Transity::Logging::LogLevel::WARN, warnMsg);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr(infoMsg));
    EXPECT_THAT(output, testing::HasSubstr(warnMsg));
    EXPECT_THAT(output, testing::Not(testing::HasSubstr(debugMsg)));
};