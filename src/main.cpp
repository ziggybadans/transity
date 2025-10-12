#include "Logger.h"
#include "app/Application.h"
#include <exception>

int main() {
    Logging::Logger logger;       // Create the logger instance
    Logging::g_logger = &logger;  // Set the global pointer

    LOG_INFO("Main", "Logger initialized.");
    logger.setLoggingEnabled(true);
    logger.setMinLogLevel(Logging::LogLevel::DEBUG);
    logger.enableFileLogging(true);

    try {
        Application app;
        app.run();
    } catch (const std::exception &e) {
        LOG_FATAL("Main", "Unhandled exception: %s.", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        LOG_FATAL("Main", "Unknown unhandled exception.");
        return EXIT_FAILURE;
    }

    return 0;
}