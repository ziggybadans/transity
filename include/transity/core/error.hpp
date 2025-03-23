// Error handling system for Transity
#pragma once

#include <stdexcept>
#include <string>

namespace transity {
namespace core {

/**
 * @brief Base class for all Transity exceptions
 */
class TransityError : public std::runtime_error {
public:
    explicit TransityError(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Exception for initialization errors
 */
class InitializationError : public TransityError {
public:
    explicit InitializationError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for runtime errors
 */
class RuntimeError : public TransityError {
public:
    explicit RuntimeError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for system-related errors
 */
class SystemError : public TransityError {
public:
    explicit SystemError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for resource-related errors
 */
class ResourceError : public TransityError {
public:
    explicit ResourceError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for state-related errors
 */
class StateError : public TransityError {
public:
    explicit StateError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for configuration errors
 */
class ConfigurationError : public TransityError {
public:
    explicit ConfigurationError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for network-related errors
 */
class NetworkError : public TransityError {
public:
    explicit NetworkError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for graphics-related errors
 */
class GraphicsError : public TransityError {
public:
    explicit GraphicsError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for input-related errors
 */
class InputError : public TransityError {
public:
    explicit InputError(const std::string& message) : TransityError(message) {}
};

/**
 * @brief Exception for recoverable errors that can be handled gracefully
 */
class RecoverableError : public TransityError {
public:
    explicit RecoverableError(const std::string& message, const std::string& recovery_hint = "")
        : TransityError(message), m_recovery_hint(recovery_hint) {}
    
    const std::string& getRecoveryHint() const { return m_recovery_hint; }

private:
    std::string m_recovery_hint;
};

} // namespace core
} // namespace transity 