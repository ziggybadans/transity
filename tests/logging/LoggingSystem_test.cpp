#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logging/LoggingSystem.h"
#include "logging/ILogger.h"

#include <string>

class MockLogger : public Transity::Logging::ILogger {
    public:
        MOCK_METHOD(void, log, 
            (Transity::Logging::LogLevel level, const std::string& message), (override));
}

TEST(LoggingSystemTest, AddAndDispatchToSingleLogger) {
    // Instantiate logging system
    Transity::Logging::LoggingSystem loggingSystem;
    // Create shared instance of logger
    auto mockLogger = std::make_shared<MockLogger>();

    // Mock method
    loggingSystem.addLogger(mockLogger);
    
    // Expect log method to be called once
    EXPECT_CALL(*mockLogger, log(Transity::Logging::LogLevel::INFO, "Test message"));
    // Call log method
    loggingSystem.log(Transity::Logging::LogLevel::INFO, "Test message");
}