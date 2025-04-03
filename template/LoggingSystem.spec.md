// File: specs/LoggingSystem.spec.md
// Module: LoggingSystem
// Description: Handles application-wide logging to various outputs (console, file).

# Specification: Logging System

## 1. Overview

Provides a centralized mechanism for logging messages with different severity levels throughout the application. Supports multiple output sinks (e.g., console, file).

## 2. Dependencies

- (Potentially) `ConfigSystem`: To load logging configuration (e.g., log level, file path).

## 3. Data Structures

- `LogLevel`: Enum (e.g., `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`)
- `LogSink`: Interface/Abstract class defining a destination for log messages (e.g., `ConsoleSink`, `FileSink`).
- `LogConfig`: Struct holding configuration like minimum log level, file path, format string.

## 4. Functions / Methods

### `initialize(config)`

1.  **Load Configuration:** Read logging settings from `config` (passed from `ConfigSystem` or defaults).
    - Determine minimum `LogLevel` to output.
    - Determine active `LogSink`s (e.g., console, file).
    - Get log file path if file logging is enabled.
    - Get log message format string.
    - `TDD_ANCHOR: Test_Logging_Config_Defaults`
    - `TDD_ANCHOR: Test_Logging_Config_LoadFromFile`
2.  **Setup Sinks:** Instantiate and configure the required `LogSink` objects.
    - If file sink is enabled, open/create the log file. Handle potential file access errors.
    - `TDD_ANCHOR: Test_Logging_ConsoleSink_Initialization`
    - `TDD_ANCHOR: Test_Logging_FileSink_Initialization`
    - `TDD_ANCHOR: Test_Logging_FileSink_ErrorHandling`
3.  **Store Configuration:** Keep track of the minimum log level and configured sinks.
4.  **Log Initialization:** Log a message indicating the logging system is initialized (e.g., "Logging system started. Level: INFO. Sinks: Console, File."). This message should go to the newly configured sinks.
    - `TDD_ANCHOR: Test_Logging_Initialization_Message`

### `log(level, message, ...args)`

1.  **Check Level:** Compare the provided `level` with the configured minimum `LogLevel`. If the message level is lower than the minimum, return immediately.
    - `TDD_ANCHOR: Test_Logging_Level_Filtering`
2.  **Format Message:** Format the `message` string with any provided `args`, and prepend timestamp, log level indicator, and potentially thread ID according to the configured format string.
    - `TDD_ANCHOR: Test_Logging_Message_Formatting`
3.  **Dispatch to Sinks:** Iterate through all active `LogSink`s and call their `write(formattedMessage)` method.
    - `TDD_ANCHOR: Test_Logging_Dispatch_To_Console`
    - `TDD_ANCHOR: Test_Logging_Dispatch_To_File`
    - `TDD_ANCHOR: Test_Logging_Dispatch_To_Multiple_Sinks`

### `shutdown()`

1.  **Log Shutdown:** Log a final message indicating shutdown.
2.  **Flush Sinks:** Ensure all buffered messages in sinks (especially file sinks) are written out.
3.  **Close Sinks:** Close any open resources (e.g., file handles).
    - `TDD_ANCHOR: Test_Logging_Shutdown_Flush`
    - `TDD_ANCHOR: Test_Logging_Shutdown_CloseFile`

### `LogSink::write(formattedMessage)` (Interface Method)

1.  **Implementation Specific:** Each concrete sink implements this.
    - `ConsoleSink`: Writes the message to `stdout` or `stderr` (perhaps based on level).
    - `FileSink`: Writes the message to the opened log file. Handles buffering internally if needed.

## 5. Helper Macros/Functions (Optional)

- Provide convenience macros like `LOG_INFO(message, ...)` which call `LoggingSystem::log(LogLevel::INFO, message, ...)` internally.
    - `TDD_ANCHOR: Test_Logging_Helper_Macros`

## 6. Edge Cases & Considerations

- **Thread Safety:** If logging can occur from multiple threads, the `log` method and sink `write` methods must be thread-safe (e.g., using mutexes).
- **Performance:** Logging, especially file I/O, can be slow. Consider asynchronous logging or buffering for performance-critical sections.
- **Configuration Errors:** Handle cases where log configuration is invalid (e.g., bad file path).
- **Log Rotation:** For long-running applications, implement log file rotation (based on size or date) to prevent excessively large files (likely out of scope for initial implementation).
- **Format String:** Define a clear syntax for the log message format string.

## 7. TDD Anchors Summary

- `Test_Logging_Config_Defaults`
- `Test_Logging_Config_LoadFromFile`
- `Test_Logging_ConsoleSink_Initialization`
- `Test_Logging_FileSink_Initialization`
- `Test_Logging_FileSink_ErrorHandling`
- `Test_Logging_Initialization_Message`
- `Test_Logging_Level_Filtering`
- `Test_Logging_Message_Formatting`
- `Test_Logging_Dispatch_To_Console`
- `Test_Logging_Dispatch_To_File`
- `Test_Logging_Dispatch_To_Multiple_Sinks`
- `Test_Logging_Shutdown_Flush`
- `Test_Logging_Shutdown_CloseFile`
- `Test_Logging_Helper_Macros`