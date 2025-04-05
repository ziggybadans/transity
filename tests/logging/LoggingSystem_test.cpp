#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logging/LoggingSystem.h"
#include "logging/ILogger.h"

#include <string>

class MockLogger : public Transity::Logging::ILogger {
    public:
        MOCK_METHOD(void, log, 
            (Transity::Logging::LogLevel level, const std::string& message), (override));
};

class LoggingSystemSingletonTest : public ::testing::Test {
    protected:   
        // Per-test set-up
        void SetUp() override {
    #ifdef ENABLE_TESTING
            // Reset the singleton before each test in this fixture
            Transity::Logging::LoggingSystem::reset();
    #endif
        }
};

TEST_F(LoggingSystemSingletonTest, SingletonAccessAndDispatch) {
    // Arrange
    auto& loggingSystem = Transity::Logging::LoggingSystem::getInstance();
    auto mockLogger = std::make_shared<MockLogger>();
    loggingSystem.addLogger(mockLogger);
    // Assert
    EXPECT_CALL(*mockLogger, log(Transity::Logging::LogLevel::INFO, "Singleton test"));
    // Act
    loggingSystem.log(Transity::Logging::LogLevel::INFO, "Singleton test");
}

TEST_F(LoggingSystemSingletonTest, AddAndDispatchToMultipleLoggersGlobal) {
    // Arrange
    auto& loggingSystem = Transity::Logging::LoggingSystem::getInstance();
    auto mockLogger1 = std::make_shared<MockLogger>();
    auto mockLogger2 = std::make_shared<MockLogger>();
    loggingSystem.addLogger(mockLogger1);
    loggingSystem.addLogger(mockLogger2);

    // Assert
    EXPECT_CALL(*mockLogger1, log(Transity::Logging::LogLevel::WARN, "Another test"));
    EXPECT_CALL(*mockLogger2, log(Transity::Logging::LogLevel::WARN, "Another test"));

    // Act
    loggingSystem.log(Transity::Logging::LogLevel::WARN, "Another test");
}

TEST(LoggingSystemTest, SingletonReturnsSameInstance) {
    // Arrange
    auto& loggingSystem1 = Transity::Logging::LoggingSystem::getInstance();
    auto& loggingSystem2 = Transity::Logging::LoggingSystem::getInstance();
    // Assert
    ASSERT_EQ(&loggingSystem1, &loggingSystem2);
}

TEST(LoggingSystemTest, AddAndDispatchToSingleLogger) {
    // Arrange
    Transity::Logging::LoggingSystem loggingSystem;
    auto mockLogger = std::make_shared<MockLogger>();
    loggingSystem.addLogger(mockLogger);
    // Assert
    EXPECT_CALL(*mockLogger, log(Transity::Logging::LogLevel::INFO, "Test message"));
    // Act
    loggingSystem.log(Transity::Logging::LogLevel::INFO, "Test message");
}

TEST(LoggingSystemTest, AddAndDispatchToMultipleLoggersLocal) {
    // Arrange
    Transity::Logging::LoggingSystem loggingSystem;
    auto mockLogger1 = std::make_shared<MockLogger>();
    auto mockLogger2 = std::make_shared<MockLogger>();
    loggingSystem.addLogger(mockLogger1);
    loggingSystem.addLogger(mockLogger2);

    // Assert
    EXPECT_CALL(*mockLogger1, log(Transity::Logging::LogLevel::INFO, "Test message"));
    EXPECT_CALL(*mockLogger2, log(Transity::Logging::LogLevel::INFO, "Test message"));
    // Act
    loggingSystem.log(Transity::Logging::LogLevel::INFO, "Test message");
}