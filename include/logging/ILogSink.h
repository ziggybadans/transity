/**
 * @file ILogSink.h
 * @brief Interface for log sink implementations
 *
 * Defines the abstract base class for all log sinks.
 * Implementations must provide write() and flush() methods.
 */
#pragma once
#include <string>

namespace transity::logging {

/**
 * @class ILogSink
 * @brief Abstract base class for log output destinations
 *
 * Implement this interface to create custom log sinks.
 * All methods must be thread-safe as they may be called
 * concurrently from multiple threads.
 */
class ILogSink {
public:
    /**
     * @brief Write a log message to the sink
     * @param message Formatted log message to output
     * @note Implementations must be thread-safe
     */
    virtual void write(const std::string& message) = 0;
    /**
     * @brief Flush any buffered log messages
     * @note Implementations must be thread-safe
     */
    virtual void flush() = 0;
    /**
     * @brief Virtual destructor
     * @details Ensures proper cleanup of derived classes
     */
    virtual ~ILogSink() = default;
};

}