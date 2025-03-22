#include "transity/core/application.hpp"
#include <iostream>

namespace transity {
namespace core {

Application::Application()
    : m_initialized(false)
    , m_appName("")
{
}

Application::~Application() {
    if (m_initialized) {
        shutdown();
    }
}

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

void Application::initialize(const std::string& appName) {
    if (m_initialized) {
        throw ApplicationError("Application is already initialized");
    }

    try {
        m_appName = appName;
        
        // TODO: Initialize subsystems here
        
        m_initialized = true;
        std::cout << "Application '" << m_appName << "' initialized successfully" << std::endl;
    }
    catch (const std::exception& e) {
        throw ApplicationError(std::string("Failed to initialize application: ") + e.what());
    }
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }

    try {
        // TODO: Shutdown subsystems here in reverse order
        
        m_initialized = false;
        std::cout << "Application '" << m_appName << "' shut down successfully" << std::endl;
    }
    catch (const std::exception& e) {
        // Log error but don't throw during shutdown
        std::cerr << "Error during shutdown: " << e.what() << std::endl;
    }
}

void Application::run() {
    if (!m_initialized) {
        throw ApplicationError("Cannot run application before initialization");
    }

    // TODO: Implement application loop
    std::cout << "Application '" << m_appName << "' is running" << std::endl;
}

} // namespace core
} // namespace transity 