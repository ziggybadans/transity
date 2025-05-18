#include "Game.h"
#include "Logger.h"
#include <exception> // For std::exception
#include <cstdlib>   // For exit()

int main()
{
    Logging::Logger::getInstance().setLoggingEnabled(true);
    Logging::Logger::getInstance().setMinLogLevel(Logging::LogLevel::TRACE);

    Logging::Logger::getInstance().enableFileLogging(true);
    LOG_INFO("Main", "Logger initialized. Enabled: %s, MinLevel: %s", Logging::Logger::getInstance().isLoggingEnabled() ? "true" : "false", Logging::Logger::getInstance().logLevelToString(Logging::Logger::getInstance().getMinLogLevel()));
    LOG_INFO("Main", "Default global log delay set to: %ums", Logging::Logger::getInstance().getLogDelay());
    LOG_INFO("Main", "Application starting.");

    try {
        Game game;
        game.init(); // Game::init() already has fatal error handling for its critical parts
        Logging::Logger::getInstance().setLogLevelDelay(Logging::LogLevel::TRACE, 2000);
        LOG_INFO("Main", "TRACE log delay set to: %ums", Logging::Logger::getInstance().getLogLevelDelay(Logging::LogLevel::TRACE));
        game.run();
    } catch (const std::exception& e) {
        // Use LOG_FATAL, which should handle logging. If LOG_FATAL itself fails, not much can be done.
        LOG_FATAL("Main", "Unhandled exception during game initialization or execution: %s. Application will terminate.", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        // Use LOG_FATAL.
        LOG_FATAL("Main", "Unknown unhandled exception during game initialization or execution. Application will terminate.");
        return EXIT_FAILURE;
    }

    LOG_INFO("Main", "Application shutting down.");
    return 0;
}