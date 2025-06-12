#include "Game.h"
#include "Logger.h"
#include <exception>
#include <cstdlib>

int main()
{
    Logging::Logger::getInstance().setLoggingEnabled(true);
    Logging::Logger::getInstance().setMinLogLevel(Logging::LogLevel::DEBUG);
    Logging::Logger::getInstance().enableFileLogging(true);

    try {
        Game game;
        game.init();
        game.run();
    } catch (const std::exception& e) {
        LOG_FATAL("Main", "Unhandled exception during game initialization or execution: %s. Application will terminate.", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        LOG_FATAL("Main", "Unknown unhandled exception during game initialization or execution. Application will terminate.");
        return EXIT_FAILURE;
    }

    return 0;
}