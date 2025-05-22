# Refactoring Plan: "Create Lines Connecting Stations"

**Overall Goals:**
1.  **Decentralize Logic from `Game`**: Move line creation logic into a dedicated system.
2.  **Improve State Management**: Use ECS-idiomatic ways to manage the state of the line being created.
3.  **Decouple UI/Input from Game Logic**: Use events or signals for communication.
4.  **Adhere to `DESIGN_ECS.md`**: Implement a `LineManagementSystem` and leverage EnTT features.
5.  **Maintain Functionality & Appearance**: Ensure no change to how the feature works or looks to the user.

---

### 1. Introduce `LineCreationSystem`

*   **Rationale**:
    *   Addresses the high coupling in the `Game` class by encapsulating the logic for creating and managing new lines.
    *   Aligns with `DESIGN_ECS.md` (Section IV, item 3: `LineManagementSystem`). We'll call it `LineCreationSystem` to be specific to this feature's interactive part.
    *   Reduces complexity in `Game::processInputCommands`.
*   **Responsibilities**:
    *   Managing the temporary state of stations selected for a new line.
    *   Handling the finalization of a new line (creating the `LineComponent` entity, linking stations).
    *   Responding to events/intents for adding stations and finalizing lines.
*   **Affected Files/Classes**:
    *   New: `src/LineCreationSystem.h`, `src/LineCreationSystem.cpp`
    *   Modified: `src/Game.h`, `src/Game.cpp` (to integrate and call the new system)
*   **Specific Changes**:
    *   Create `LineCreationSystem` class.
    *   It will hold the temporary list of stations for the new line (e.g., `std::vector<entt::entity> m_stationsForCurrentLine;`).
    *   Methods:
        *   `void addStationToLine(entt::entity stationEntity);`
        *   `void finalizeLine(entt::registry& registry);` (This will contain the logic currently in `Game::processInputCommands` for `FinalizeLineIntent`).
        *   `void clearCurrentLine();`
        *   `const std::vector<entt::entity>& getActiveLineStations() const;` (for rendering)
        *   `void update(entt::registry& registry /*, other necessary inputs */);` (called by `Game` in its update loop).
    *   `Game` will instantiate and call `LineCreationSystem::update()`. The `m_stationsForNewLine` vector will be removed from `Game`.

---

### 2. Introduce `ActiveLineStationComponent` (Tag)

*   **Rationale**:
    *   Replaces the `Game::m_stationsForNewLine` vector (and the proposed `LineCreationSystem::m_stationsForCurrentLine`) with an ECS-idiomatic approach.
    *   Aligns with `DESIGN_ECS.md` (Appendix A on Tags). This component will act as a tag.
    *   Simplifies querying for stations part of the active line, especially for rendering.
*   **Definition**:
    *   `struct ActiveLineStationComponent { int order = 0; };` // `order` to maintain sequence.
*   **Affected Files/Classes**:
    *   Modified: `src/Components.h` (to add the new component)
    *   Modified: `src/LineCreationSystem.cpp` (to use this component instead of a vector)
    *   Modified: `src/Renderer.cpp` (to query for this component)
    *   Modified: `src/Game.cpp` (no longer passes the vector to `Renderer`)
*   **Specific Changes**:
    *   When `LineCreationSystem::addStationToLine` is called, it will add `ActiveLineStationComponent` to the station entity and set its `order`.
    *   When `LineCreationSystem::finalizeLine` or `clearCurrentLine` is called, it will remove `ActiveLineStationComponent` from all relevant station entities.
    *   `Renderer::render` will query the registry for entities with `PositionComponent` and `ActiveLineStationComponent`, sort them by `order`, and then draw the active line segments and the line to the mouse. This removes the need for `Game` to pass `activeLineStations` vector.

---

### 3. Refactor Input Handling and UI Interaction using Events/Signals

*   **Rationale**:
    *   Decouples `Game` from `UI::wasFinalizeLineClicked` and `InputHandler`'s command queue for line-specific intents.
    *   Promotes a more event-driven architecture, aligning with `DESIGN_ECS.md` (Section V on EnTT Observers/Signals, or a simple event struct).
*   **Affected Files/Classes**:
    *   Modified: `src/InputHandler.h`, `src/InputHandler.cpp`
    *   Modified: `src/UI.h`, `src/UI.cpp`
    *   Modified: `src/LineCreationSystem.h`, `src/LineCreationSystem.cpp`
    *   Modified: `src/Game.cpp` (to set up event handling or pass events to `LineCreationSystem`)
*   **Specific Changes (Option A: Simple Event Structs)**:
    *   Define event structs:
        ```cpp
        // In a new file like src/LineEvents.h or within InputHandler.h
        struct AddStationToLineEvent {
            entt::entity stationEntity;
        };
        struct FinalizeLineEvent {};
        ```
    *   `InputHandler::handleGameEvent`: When a station is clicked in `CREATE_LINE` mode, instead of creating an `AddStationToLineIntent` command, it will generate an `AddStationToLineEvent`.
    *   `UI::update`: When the "Finalize Line" button is clicked, it will generate a `FinalizeLineEvent`.
    *   `LineCreationSystem::update`: Will take a list of these events (e.g., passed from `Game` which collects them from `InputHandler` and `UI` each frame) and process them.
        *   On `AddStationToLineEvent`, call its internal logic to add the station (which now means adding `ActiveLineStationComponent`).
        *   On `FinalizeLineEvent`, call its internal `finalizeLine` method.
    *   `Game::processInputCommands` will no longer handle `AddStationToLineIntent` or `FinalizeLineIntent` directly from `m_inputHandler->getCommands()`. It will gather events from UI and InputHandler and pass them to `LineCreationSystem`.
*   **Specific Changes (Option B: EnTT Signals/Observer - More Advanced)**:
    *   `InputHandler` and `UI` could directly emit signals (e.g., using `entt::sigh`) or `LineCreationSystem` could observe component changes if intents were modeled as temporary components. For this stage, simple event structs (Option A) might be less invasive and easier to implement while still achieving decoupling. The plan will proceed with Option A for clarity, but Option B is a valid alternative.

---

### 4. Move Line Entity Creation to `EntityFactory` (or keep in `LineCreationSystem`)

*   **Rationale**:
    *   `DESIGN_ECS.md` (Section VI) suggests `EntityFactory` should be adapted for creating pre-configured entities.
    *   Centralizes entity creation logic.
*   **Affected Files/Classes**:
    *   Modified: `src/EntityFactory.h`, `src/EntityFactory.cpp`
    *   Modified: `src/LineCreationSystem.cpp`
*   **Specific Changes**:
    *   Add `entt::entity EntityFactory::createLine(const std::vector<entt::entity>& stops, sf::Color color);`
    *   This method in `EntityFactory` will:
        *   Create a new entity.
        *   Emplace `LineComponent` with the given stops and color.
        *   Potentially emplace other common components for lines if any (e.g., a `RenderableComponent` if lines themselves become more complex renderable objects beyond simple vertex arrays drawn by the `Renderer`).
    *   `LineCreationSystem::finalizeLine` will call `m_entityFactory.createLine(...)` instead of directly manipulating the registry to create the line entity. It will still be responsible for updating the `StationComponent::connectedLines` on each station.
    *   *Alternative*: The `LineCreationSystem` could retain the responsibility for creating the line entity if a line is considered a direct product of that system's logic rather than a generic "archetype" from the factory. For now, moving to `EntityFactory` is slightly cleaner for centralizing creation.

---

### 5. Refine `Renderer::render` for Active Line

*   **Rationale**:
    *   To use the new `ActiveLineStationComponent` tag.
    *   Reduces direct data passing from `Game`.
*   **Affected Files/Classes**:
    *   Modified: `src/Renderer.h`, `src/Renderer.cpp`
    *   Modified: `src/Game.cpp` (no longer passes `activeLineStations` vector or `InteractionMode` if the renderer can infer mode or if active line rendering is solely based on component presence).
*   **Specific Changes**:
    *   `Renderer::render` method signature changes:
        `void render(entt::registry& registry, const sf::View& view, sf::Time dt /*, InteractionMode currentMode - potentially removed if not needed for other things */);`
    *   Inside `Renderer::render`:
        *   To draw the active line:
            *   Create a view for entities with `PositionComponent` and `ActiveLineStationComponent`.
            *   Collect these entities.
            *   Sort them by `ActiveLineStationComponent::order`.
            *   Draw segments between these sorted stations (using a distinct color like yellow).
            *   Draw a segment from the last station in the sorted list to the current mouse position (if the list is not empty and `InteractionMode` is `CREATE_LINE`. The renderer might still need `InteractionMode` or a similar flag/component to know if it *should* draw the line-to-mouse).
        *   The `InteractionMode` might still be needed by the renderer if other rendering logic depends on it, or if the "draw line to mouse" behavior is strictly tied to being in `CREATE_LINE` mode and not just the presence of `ActiveLineStationComponent`s. A `CurrentlyCreatingLineTag` on a global game state entity or a context variable in the registry could also signal this. For simplicity, keeping `InteractionMode` in `Renderer::render` for now is acceptable if it's used for more than just the active line.

---

### 6. Ensure Functionality and Visual Consistency

*   **Visuals**:
    *   The finalized lines must still be drawn based on `LineComponent` data (color, stops) as they are now. This is largely unaffected by the refactoring of the *creation* process.
    *   The "in-progress" line (segments between selected stations and the line to the mouse) must look and behave identically. Using `ActiveLineStationComponent` with an `order` member and querying it in the `Renderer` will achieve this. The line to the mouse cursor will still be drawn from the last station with `ActiveLineStationComponent` (highest `order`).
*   **Functionality**:
    *   Clicking stations in `CREATE_LINE` mode adds them to the line.
    *   The "Finalize Line" button appears only when >= 2 stations are selected.
    *   Clicking "Finalize Line" creates the permanent line, updates station connections, and clears the temporary line.
    *   The correct `InteractionMode` must still be passed around or made available where decisions depend on it (e.g., `InputHandler` for enabling station clicks, `UI` for button visibility, `Renderer` for drawing line-to-mouse).

---

### Summary of Affected Files and High-Level Changes:

*   **`src/Components.h`**:
    *   Add `ActiveLineStationComponent { int order; };`
*   **`src/LineCreationSystem.h` / `src/LineCreationSystem.cpp` (New Files)**:
    *   Manages adding/removing `ActiveLineStationComponent`.
    *   Handles `FinalizeLineEvent` to create the line (calling `EntityFactory`).
    *   Updates `StationComponent::connectedLines`.
*   **`src/Game.h` / `src/Game.cpp`**:
    *   Remove `m_stationsForNewLine`.
    *   Instantiate and update `LineCreationSystem`.
    *   Relay events (or provide access to event queues) from `InputHandler` and `UI` to `LineCreationSystem`.
    *   No longer directly passes `m_stationsForNewLine` or `InteractionMode` for active line rendering if `Renderer` can derive it.
*   **`src/InputHandler.h` / `src/InputHandler.cpp`**:
    *   On station click in `CREATE_LINE` mode, generate `AddStationToLineEvent` instead of `AddStationToLineIntent` command.
    *   May need a way to publish these events (e.g., a getter for an event queue).
*   **`src/UI.h` / `src/UI.cpp`**:
    *   On "Finalize Line" button click, generate `FinalizeLineEvent`.
    *   `update` method signature might change if `numStationsInActiveLine` is now queried differently (e.g., from `LineCreationSystem` or by querying registry for `ActiveLineStationComponent`).
*   **`src/Renderer.h` / `src/Renderer.cpp`**:
    *   Modify `render` to query for `ActiveLineStationComponent` to draw the active line.
    *   Sort by `order` to draw segments correctly.
    *   Draw line to mouse from the last active station.
*   **`src/EntityFactory.h` / `src/EntityFactory.cpp`**:
    *   Add `createLine(const std::vector<entt::entity>& stops, sf::Color color)` method.
*   **`src/LineEvents.h` (New File or part of existing header)**:
    *   Define `AddStationToLineEvent` and `FinalizeLineEvent` structs.

---