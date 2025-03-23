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

} // namespace core
} // namespace transity 