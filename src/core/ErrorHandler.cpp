#include "ErrorHandler.h"
#include <algorithm>

// Initialize the static member
std::vector<ErrorHandler::ErrorCallback> ErrorHandler::m_handlers;

void ErrorHandler::RegisterHandler(ErrorCallback handler) {
    if (handler) {
        m_handlers.push_back(std::move(handler));
    }
}

void ErrorHandler::ReportError(ErrorSeverity severity, const std::string& message) {
    // Make a copy of the handlers vector to avoid issues if handlers modify the vector
    auto handlers = m_handlers;
    
    // Notify all registered handlers
    for (const auto& handler : handlers) {
        try {
            handler(severity, message);
        } catch (...) {
            // Prevent exceptions in handlers from affecting other handlers
            continue;
        }
    }
}

void ErrorHandler::ClearHandlers() {
    m_handlers.clear();
} 