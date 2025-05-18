#include "Game.h"
#include "Logger.h"
#include <exception> // For std::exception
#include <cstdlib>   // For exit()

int main()
{
    Logging::Logger::getInstance().setLoggingEnabled(true);
    Logging::Logger::getInstance().setMinLogLevel(Logging::LogLevel::TRACE);

    Logging::Logger::getInstance().enableFileLogging(true);

    try {
        Game game;
        game.init();
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

    return 0;
}