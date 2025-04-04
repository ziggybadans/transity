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

**Objective 2: Implement Logging System**

*   **Task 2.1: Define Logging Interface (`ILogger`)**
    *   Steps:
    ✓   1.  Define an abstract base class or interface (`ILogger`) with methods for different log levels (e.g., `debug`, `info`, `warn`, `error`).
        2.  Ensure the interface is simple and decoupled from specific implementations.
*   **Task 2.2: Implement Console Logger**
    *   Steps:
        1.  Write unit tests (TDD) for basic logging functionality (output format, level filtering).
        2.  Create a concrete `ConsoleLogger` class implementing `ILogger`.
        3.  Implement logging to `stdout`/`stderr`.
        4.  Add timestamp and log level formatting.
*   **Task 2.3: Implement File Logger**
    *   Steps:
        1.  Write unit tests (TDD) for file logging (file creation, writing, rotation - if needed later).
        2.  Create a concrete `FileLogger` class implementing `ILogger`.
        3.  Implement logging to a specified file.
*   **Task 2.4: Create Logging System Facade (`LoggingSystem`)**
    *   Steps:
        1.  Write unit tests (TDD) for managing multiple loggers and providing a global access point (e.g., singleton or service locator).
        2.  Implement `LoggingSystem` to manage logger instances (e.g., allow adding/removing console/file loggers).
        3.  Provide static methods or a globally accessible instance for easy logging throughout the application.
        4.  Integrate basic logging calls into the initial project setup to test.

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