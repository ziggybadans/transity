#include <gtest/gtest.h>

#include "logging/logging_system.h"


TEST(LoggingSystem, InitializesWithDefaultConfig) {
    transity::logging::LoggingSystem logger;

    logger.initialize();

    ASSERT_EQ(logger.getLogLevel(), transity::logging::LogLevel::INFO);
    ASSERT_TRUE(logger.isConsoleSinkEnabled());
    ASSERT_FALSE(logger.isFileSinkEnabled());
}

TEST(LoggingSystem, InitializesWithCustomConfig) {
    transity::logging::LoggingSystem logger;
    transity::logging::LogLevel customLevel = transity::logging::LogLevel::DEBUG;
    bool enableFileSink = true;
    std::string filePath = "custom_log.txt";
    bool enableConsoleSink = false;

    logger.initialize(customLevel, enableFileSink, enableConsoleSink, filePath);

    ASSERT_EQ(logger.getLogLevel(), customLevel);
    ASSERT_EQ(logger.isFileSinkEnabled(), enableFileSink);
    ASSERT_EQ(logger.isConsoleSinkEnabled(), enableConsoleSink);
    ASSERT_EQ(logger.getFilePath(), filePath);
}

TEST(LoggingSystem, ConsoleSinkInitializes) {
    transity::logging::LoggingSystem logger;

    logger.initialize(transity::logging::LogLevel::INFO, false, true);

    ASSERT_TRUE(logger.isConsoleSinkEnabled());
}

TEST(LoggingSystem, FileSinkInitializes) {
    transity::logging::LoggingSystem logger;

    logger.initialize(transity::logging::LogLevel::INFO, true, false);

    ASSERT_TRUE(logger.isFileSinkEnabled());
}

TEST(LoggingSystem, FileSinkHandlesErrors) {
    transity::logging::LoggingSystem logger;

    ASSERT_THROW(
        logger.initialize(transity::logging::LogLevel::INFO, true, false, "invalid/path.txt"),
        std::runtime_error
    );
}