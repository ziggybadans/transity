// File: specs/CoreApplication.spec.md
// Module: CoreApplication
// Description: Handles the main application lifecycle, initialization, game loop, and shutdown.

# Specification: Core Application

## 1. Overview

This module defines the central orchestrator of the Transity application. It is responsible for:
- Initializing all core systems (ECS, Rendering, Input, Config, Logging).
- Running the main game loop.
- Handling application shutdown and cleanup.

## 2. Dependencies

- `ECSCore`: For managing entities, components, and systems.
- `RenderingSystem`: For window creation and drawing.
- `InputSystem`: For processing user input.
- `ConfigSystem`: For loading application settings.
- `LoggingSystem`: For logging messages.

## 3. Data Structures

- `ApplicationState`: Enum (e.g., `INITIALIZING`, `RUNNING`, `SHUTTING_DOWN`)
- `CoreSystems`: Struct/Class holding references or pointers to instances of core system modules (ECS, Rendering, Input, Config, Logging).

## 4. Functions / Methods

### `main()` - Entry Point

1.  **Initialize Logging:** Set up the `LoggingSystem` first to capture logs from subsequent initializations.
    - `TDD_ANCHOR: Test_Logging_Initialization`
2.  **Initialize Configuration:** Load settings using `ConfigSystem`.
    - `TDD_ANCHOR: Test_Config_Loading_Defaults`
    - `TDD_ANCHOR: Test_Config_Loading_FromFile`
3.  **Initialize Core Systems:**
    - Instantiate `ECSCore`.
        - `TDD_ANCHOR: Test_ECSCore_Initialization`
    - Instantiate `RenderingSystem` (passing config settings like window size).
        - `TDD_ANCHOR: Test_RenderingSystem_Initialization`
        - `TDD_ANCHOR: Test_RenderingSystem_WindowCreation`
    - Instantiate `InputSystem` (potentially passing window reference from RenderingSystem).
        - `TDD_ANCHOR: Test_InputSystem_Initialization`
    - Store system instances (e.g., in `CoreSystems` struct).
4.  **Set Application State:** Change state to `RUNNING`.
5.  **Enter Main Loop:** Call `runMainLoop(coreSystems)`.
6.  **Shutdown:** Call `shutdown(coreSystems)` upon loop exit.
7.  **Log Exit:** Log successful application termination.
8.  **Return:** Exit application (e.g., return 0).

### `runMainLoop(coreSystems)`

1.  **Loop Condition:** While `ApplicationState` is `RUNNING` AND `RenderingSystem.isWindowOpen()`:
    - **Process Input:** Call `InputSystem.processEvents()`.
        - If `InputSystem` signals exit request (e.g., window close button), set `ApplicationState` to `SHUTTING_DOWN`.
        - `TDD_ANCHOR: Test_Input_WindowCloseEvent`
    - **Update Systems (Fixed Timestep - Optional but Recommended for Simulation):**
        - Calculate elapsed time since last update.
        - While accumulated time >= fixed timestep:
            - `ECSCore.updateSystems(fixedTimestep)` // Or call specific systems in order
            - Subtract fixed timestep from accumulator.
            - `TDD_ANCHOR: Test_FixedTimestep_UpdatesOccur`
    - **Update Systems (Variable Timestep - For non-physics):**
        - Calculate delta time since last frame.
        - Call update methods on relevant systems (e.g., `AnimationSystem.update(deltaTime)` if it existed).
        - `TDD_ANCHOR: Test_VariableTimestep_UpdatesOccur`
    - **Render:**
        - Call `RenderingSystem.clear()`
        - Call `ECSCore.renderSystems()` // Or call specific rendering systems
        - Call `RenderingSystem.display()`
        - `TDD_ANCHOR: Test_Rendering_FrameIsDisplayed`
2.  **Log Loop Exit:** Log reason for main loop termination (e.g., user requested exit).

### `shutdown(coreSystems)`

1.  **Log Shutdown:** Log initiation of shutdown sequence.
2.  **Cleanup Systems:** Call shutdown/cleanup methods on systems in reverse order of initialization (e.g., `InputSystem.shutdown()`, `RenderingSystem.shutdown()`, `ECSCore.shutdown()`).
    - `TDD_ANCHOR: Test_System_Shutdown_Order`
3.  **Cleanup Configuration/Logging:** Perform any final cleanup for Config and Logging systems if necessary.
4.  **Log Completion:** Log successful shutdown.

## 5. Edge Cases & Considerations

- **Initialization Failure:** How are failures during system initialization handled? (e.g., Config file not found, Rendering window fails to create). Should log errors and exit gracefully.
- **Resource Leaks:** Ensure all resources (memory, file handles, graphics resources) are released during shutdown.
- **Fixed vs. Variable Timestep:** Decide on the update strategy. A fixed timestep is generally better for physics and simulation consistency.
- **System Dependencies:** Clearly define the order of updates if systems depend on each other's state within a single frame/update cycle.

## 6. TDD Anchors Summary

- `Test_Logging_Initialization`
- `Test_Config_Loading_Defaults`
- `Test_Config_Loading_FromFile`
- `Test_ECSCore_Initialization`
- `Test_RenderingSystem_Initialization`
- `Test_RenderingSystem_WindowCreation`
- `Test_InputSystem_Initialization`
- `Test_Input_WindowCloseEvent`
- `Test_FixedTimestep_UpdatesOccur`
- `Test_VariableTimestep_UpdatesOccur`
- `Test_Rendering_FrameIsDisplayed`
- `Test_System_Shutdown_Order`