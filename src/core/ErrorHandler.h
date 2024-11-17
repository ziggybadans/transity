#pragma once

#include <string>
#include <functional>
#include <vector>

enum class ErrorSeverity {
    Info,
    Warning,
    Error,
    Critical
};

class ErrorHandler {
public:
    using ErrorCallback = std::function<void(ErrorSeverity, const std::string&)>;
    
    static void RegisterHandler(ErrorCallback handler);
    static void ReportError(ErrorSeverity severity, const std::string& message);
    static void ClearHandlers();

private:
    static std::vector<ErrorCallback> m_handlers;
}; 