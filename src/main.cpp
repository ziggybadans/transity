#include "Game.h"
#include "Logger.h"

int main()
{
    Logging::setLoggingEnabled(true);
    Logging::setMinLogLevel(Logging::LogLevel::TRACE);

    LOG_INFO("Main", "Logger initialized. Enabled: %s, MinLevel: %s", Logging::isLoggingEnabled() ? "true" : "false", Logging::logLevelToString(Logging::getMinLogLevel()));
    LOG_INFO("Main", "Default global log delay set to: %ums", Logging::getLogDelay());
    LOG_INFO("Main", "Application starting.");

    Game game;
    game.init();
    Logging::setLogLevelDelay(Logging::LogLevel::TRACE, 2000);
    LOG_INFO("Main", "TRACE log delay set to: %ums", Logging::getLogLevelDelay(Logging::LogLevel::TRACE));
    game.run();

    LOG_INFO("Main", "Application shutting down.");
    return 0;
}