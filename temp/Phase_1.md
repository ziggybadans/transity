## Phase 1: Foundation - Development Timeline Checklist

**Goal:** Establish the core application structure, essential systems (logging, config, input, rendering), and the fundamental ECS framework.

**Objective 1: Project Setup & Core Dependencies**

*   **Task 1.1: Setup Build System (CMake)**
    *   Steps:
    ✓   1.  Initialize CMake project (`CMakeLists.txt`).
    ✓   2.  Define project name, C++ standard (C++17).
    ✓   3.  Configure basic build options (Debug/Release).
    ✓   4.  Add subdirectories for source (`src`), tests (`tests`), etc.
    ✓   5.  Integrate dependency management for external libraries (SFML, EnTT, ImGui, GTest) using CMake's `find_package` or a package manager like vcpkg/Conan.
    ✓   6.  Write a simple "Hello World" test case using GTest to verify test framework integration.
    ✓   7.  Configure basic CI/CD pipeline (e.g., GitHub Actions) for automated builds and tests.
*   **Task 1.2: Establish Coding Standards & Tooling**
    *   Steps:
    ✓   1.  Define coding style guidelines (e.g., based on Google C++ Style Guide, LLVM).
    ✓   2.  Integrate static analysis tools (e.g., Clang-Tidy, Cppcheck) into the build process.
    ✓   3.  Integrate code formatting tools (e.g., Clang-Format) and enforce formatting checks in CI.
    ✓   4.  Set up `.gitignore` and `CONTRIBUTING.md`.

**Objective 2: Implement Robust Logging System (TDD)**

*   **Goal:** Establish a flexible, configurable, and thread-safe logging system using Test-Driven Development (TDD). This system will support multiple output sinks (console, file) and severity levels, adhering closely to the specifications outlined in `template/LoggingSystem.spec.md`.
*   **Core Principles:**
    *   **Modularity:** Decouple the logging interface (`LogSink`) from concrete implementations (`ConsoleSink`, `FileSink`).
    *   **Configurability:** Allow configuration of log level, output sinks, file paths, and message format via the `ConfigSystem` (or defaults).
    *   **Testability:** Ensure all components are unit-testable.
*   **TDD Workflow:** Development will strictly follow the **Red-Green-Refactor** cycle. For each feature or requirement, guided by the `TDD_ANCHOR` points defined in `template/LoggingSystem.spec.md`:
    1.  **Red:** Write a unit test that captures the requirement and fails because the functionality is not yet implemented. Commit this failing test.
    2.  **Green:** Write the simplest, minimal code necessary to make the failing test pass. Verify all tests pass. Commit the working code.
    3.  **Refactor:** Improve the code's design, clarity, performance, and adherence to coding standards *without changing its external behavior*. Ensure all tests continue to pass after refactoring. Commit the refactored code.
*   **Task 2.1: Define Core Logging Structures & Interface**
    *   **Goal:** Define the fundamental data types and the abstract interface for log outputs.
    *   **Steps:**
        1.  Define `LogLevel` enum (`TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`) based on the spec.
        2.  Define `LogSink` interface (abstract class) with a pure virtual `write(const std::string& formattedMessage)` method.
        3.  Define `LogConfig` struct to hold configuration parameters (minimum `LogLevel`, file path, format string, active sinks).
*   **Task 2.2: Implement Console Sink (`ConsoleSink`)**
    *   **Goal:** Create a concrete `LogSink` that writes messages to the standard console output (`stdout`/`stderr`).
    *   **Steps (following TDD workflow):**
        1.  Implement `ConsoleSink` inheriting from `LogSink`.
        2.  Implement the `write` method to output formatted messages to the console.
        3.  *Targeted TDD Anchors:* `Test_Logging_ConsoleSink_Initialization`, `Test_Logging_Dispatch_To_Console`.
*   **Task 2.3: Implement File Sink (`FileSink`)**
    *   **Goal:** Create a concrete `LogSink` that writes messages to a specified log file.
    *   **Steps (following TDD workflow):**
        1.  Implement `FileSink` inheriting from `LogSink`.
        2.  Implement constructor to open/create the log file based on configuration.
        3.  Implement the `write` method to append formatted messages to the file stream.
        4.  Implement destructor or `shutdown` method to ensure the file stream is flushed and closed properly.
        5.  Handle potential file I/O errors gracefully (e.g., log to console if file fails).
        6.  *Targeted TDD Anchors:* `Test_Logging_FileSink_Initialization`, `Test_Logging_FileSink_ErrorHandling`, `Test_Logging_Dispatch_To_File`, `Test_Logging_Shutdown_Flush`, `Test_Logging_Shutdown_CloseFile`.
*   **Task 2.4: Implement Logging System Facade (`LoggingSystem`)**
    *   **Goal:** Create a central manager for configuring, initializing, and dispatching log messages to active sinks.
    *   **Steps (following TDD workflow):**
        1.  Implement `LoggingSystem` class (consider singleton or service locator pattern for access).
        2.  Implement `initialize(const LogConfig& config)`:
            *   Store configuration (min level, format).
            *   Instantiate and store configured `LogSink` objects (Console, File).
            *   Log an initialization message via the newly configured sinks.
            *   *Targeted TDD Anchors:* `Test_Logging_Config_Defaults`, `Test_Logging_Config_LoadFromFile` (assuming integration with `ConfigSystem`), `Test_Logging_Initialization_Message`, `Test_Logging_Dispatch_To_Multiple_Sinks`.
        3.  Implement `log(LogLevel level, const std::string& message, ... /* variadic args or fmtlib */)`:
            *   Check if `level` meets the minimum configured threshold.
            *   Format the message string (timestamp, level, message content) according to `LogConfig`.
            *   Dispatch the formatted message to all active `LogSink`s. Ensure thread safety if logging from multiple threads is anticipated.
            *   *Targeted TDD Anchors:* `Test_Logging_Level_Filtering`, `Test_Logging_Message_Formatting`.
        4.  Implement `shutdown()`:
            *   Log a shutdown message.
            *   Call shutdown/cleanup methods on all active sinks (e.g., flush/close `FileSink`).
            *   *Targeted TDD Anchors:* (Covered by sink tests like `Test_Logging_Shutdown_Flush`, `Test_Logging_Shutdown_CloseFile`).
        5.  (Optional) Implement helper macros (e.g., `LOG_INFO(...)`, `LOG_WARN(...)`) for convenient logging calls.
            *   *Targeted TDD Anchors:* `Test_Logging_Helper_Macros`.
*   **Task 2.5: Integrate Basic Logging into `CoreApplication`**
    *   **Goal:** Connect the `LoggingSystem` to the application's lifecycle.
    *   **Steps:**
        1.  In `CoreApplication::initialize()`, create `LogConfig` (potentially loading from `ConfigSystem`) and call `LoggingSystem::initialize()`.
        2.  In `CoreApplication::shutdown()`, call `LoggingSystem::shutdown()`.
        3.  Add basic `LOG_INFO` calls at key points (e.g., "Initializing RenderingSystem...", "Application shutdown.") to verify integration.

**Objective 3: Implement Configuration System**

*   **Task 3.1: Define Configuration Interface (`IConfigProvider`)**
    *   Steps:
        1.  Define an interface for loading and accessing configuration data (e.g., `getValue<T>(key)`, `hasKey(key)`).
        2.  Consider support for different data types (int, float, string, bool).
*   **Task 3.2: Implement INI/JSON File Config Provider**
    *   Steps:
        1.  Choose a config file format (e.g., INI or JSON). Select a suitable C++ library if needed.
        2.  Write unit tests (TDD) for loading files, parsing key-value pairs, handling missing files/keys, and type conversions.
        3.  Implement a concrete `FileConfigProvider` class implementing `IConfigProvider`.
*   **Task 3.3: Create Configuration System Facade (`ConfigSystem`)**
    *   Steps:
        1.  Write unit tests (TDD) for managing config providers and providing unified access.
        2.  Implement `ConfigSystem` to load configuration from specified sources (e.g., default file, user overrides).
        3.  Provide a global access point or service locator pattern.
        4.  Define initial configuration keys needed (e.g., window width/height, title).

**Objective 4: Implement Core ECS Framework (`ECSCore`)**

*   **Task 4.1: Integrate EnTT Library**
    *   Steps:
        1.  Ensure EnTT is correctly linked via CMake.
        2.  Write basic unit tests (TDD) to verify core EnTT functionality: creating/destroying entities, adding/removing components, iterating views.
*   **Task 4.2: Define Core Component Types**
    *   Steps:
        1.  Define the `PositionComponent` struct (as per `PositionComponent.spec.md`). Ensure it's a simple Plain Old Data (POD) struct.
        2.  Write unit tests (TDD) for component creation and data access.
*   **Task 4.3: Implement `ECSCore` Wrapper/Manager**
    *   Steps:
        1.  Define the `ECSCore` class interface based on `ECSCore.spec.md`.
        2.  Write unit tests (TDD) for `ECSCore` responsibilities: managing the EnTT registry, providing simplified entity/component manipulation methods, registering/updating/rendering systems (placeholders for now).
        3.  Implement `ECSCore` using the EnTT registry.
        4.  Provide access to the core registry.

**Objective 5: Implement Rendering System (`RenderingSystem`)**

*   **Task 5.1: Integrate SFML Library**
    *   Steps:
        1.  Ensure SFML (graphics, window, system modules) is correctly linked via CMake.
        2.  Write basic tests (may require visual inspection or mock objects) to create an SFML window.
*   **Task 5.2: Define Rendering Interface (`IRenderer`)**
    *   Steps:
        1.  Define an interface with drawing primitives (e.g., `drawCircle`, `drawLine`, `drawText`, `drawSprite`).
        2.  Include methods for view/camera manipulation (e.g., `setCenter`, `setZoom`, `getViewport`).
        3.  Include methods for coordinate transformations (world to screen, screen to world).
*   **Task 5.3: Implement `RenderingSystem`**
    *   Steps:
        1.  Define the `RenderingSystem` class interface based on `RenderingSystem.spec.md`.
        2.  Write unit tests (TDD, potentially using mocks for `sf::RenderWindow`) for window creation based on `ConfigSystem`, event polling, clearing the screen, displaying content.
        3.  Implement `RenderingSystem` to manage the `sf::RenderWindow`.
        4.  Implement the `IRenderer` interface using SFML draw calls.
        5.  Handle window events (close, resize) and potentially forward them.
        6.  Integrate with `ConfigSystem` to get window settings.
        7.  Integrate with `LoggingSystem` for reporting errors.

**Objective 6: Implement Input System (`InputSystem`)**

*   **Task 6.1: Define Input Abstractions**
    *   Steps:
        1.  Define enums or constants for keyboard keys, mouse buttons, and potentially game actions (e.g., `Action::PanCamera`, `Action::Select`).
        2.  Define structures to hold input state (e.g., mouse position, key/button down/pressed/released status).
*   **Task 6.2: Implement `InputSystem`**
    *   Steps:
        1.  Define the `InputSystem` class interface based on `InputSystem.spec.md`.
        2.  Write unit tests (TDD) for processing raw SFML events, updating internal state (key/button status, mouse position), checking action states (`isActionPressed`, `isActionDown`).
        3.  Implement `InputSystem` to receive events from `RenderingSystem`.
        4.  Implement logic to map raw inputs to game actions (initially hardcoded, later driven by `ConfigSystem`).
        5.  Implement methods to query input state and action states.
        6.  Integrate with `LoggingSystem`.
        7.  (Placeholder for ImGui focus handling).

**Objective 7: Implement Core Application (`CoreApplication`)**

*   **Task 7.1: Define Application Lifecycle**
    *   Steps:
        1.  Define methods for `initialize()`, `run()`, `shutdown()`.
*   **Task 7.2: Implement `CoreApplication`**
    *   Steps:
        1.  Define the `CoreApplication` class interface based on `CoreApplication.spec.md`.
        2.  Write integration tests (TDD style) for the main loop structure: initialization, event polling, update (placeholder), render (placeholder), shutdown.
        3.  Implement `CoreApplication` to own and manage the lifecycle of core systems (`LoggingSystem`, `ConfigSystem`, `ECSCore`, `RenderingSystem`, `InputSystem`).
        4.  Implement the main game loop:
            *   Process window events via `RenderingSystem`.
            *   Pass events to `InputSystem`.
            *   Update `InputSystem` state.
            *   (Placeholder for `ECSCore` system updates).
            *   Clear screen via `RenderingSystem`.
            *   (Placeholder for `ECSCore` system rendering).
            *   Display frame via `RenderingSystem`.
        5.  Implement initialization logic (create systems in correct order, load config).
        6.  Implement shutdown logic (destroy systems in reverse order).
*   **Task 7.3: Create `main.cpp` Entry Point**
    *   Steps:
        1.  Create `main.cpp`.
        2.  Instantiate `CoreApplication`.
        3.  Call `initialize()`, `run()`, `shutdown()`.
        4.  Include basic error handling (e.g., try-catch around `run`).
        5.  Compile and run the application (should show a blank window that can be closed).