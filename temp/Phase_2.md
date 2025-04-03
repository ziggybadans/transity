## Phase 2: Mini Metro Clone - Development Timeline Checklist

**Goal:** Implement the Minimum Viable Product (MVP) replicating core Mini Metro gameplay using the established foundation.

**Objective 1: Implement Core Gameplay Components**

*   **Task 1.1: Implement `StationComponent`**
    *   Steps:
        1.  Define `StationComponent` struct based on `StationComponent.spec.md` (e.g., position, shape type, passenger queue, capacity). Ensure POD.
        2.  Write unit tests (TDD) for component data.
*   **Task 1.2: Implement `PassengerComponent`**
    *   Steps:
        1.  Define `PassengerComponent` struct based on `PassengerComponent.spec.md` (e.g., origin station entity, destination station shape type, current state - waiting/travelling). Ensure POD.
        2.  Write unit tests (TDD) for component data.
*   **Task 1.3: Implement `LineComponent`**
    *   Steps:
        1.  Define `LineComponent` struct based on `LineComponent.spec.md` (e.g., list of station entities, color, associated vehicle entities). Ensure POD.
        2.  Write unit tests (TDD) for component data, including adding/removing stations.
*   **Task 1.4: Implement `VehicleComponent`**
    *   Steps:
        1.  Define `VehicleComponent` struct based on `VehicleComponent.spec.md` (e.g., current position, associated line entity, target station entity, passenger list, capacity, speed). Ensure POD.
        2.  Write unit tests (TDD) for component data.

**Objective 2: Implement Game Setup & Initial State**

*   **Task 2.1: Implement `GameSetupSystem`**
    *   Steps:
        1.  Define `GameSetupSystem` interface/class based on `GameSetupSystem.spec.md`.
        2.  Write integration tests (TDD style) verifying that the system creates the initial game state (e.g., a few stations, maybe one line) in the ECS registry upon startup.
        3.  Implement `GameSetupSystem` logic:
            *   Access `ECSCore` registry.
            *   Create entities for initial stations with `PositionComponent` and `StationComponent`.
            *   (Optionally) Create an initial line entity with `LineComponent`.
        4.  Register `GameSetupSystem` with `ECSCore` to run once at the start.

**Objective 3: Implement Station Rendering**

*   **Task 3.1: Implement `StationRenderSystem`**
    *   Steps:
        1.  Define `StationRenderSystem` interface/class based on `StationRenderSystem.spec.md`.
        2.  Write unit tests (TDD, potentially mocking the `IRenderer`) to verify the system queries the correct components and calls draw functions appropriately based on station shape/state.
        3.  Implement `StationRenderSystem` logic:
            *   Access `ECSCore` registry and `IRenderer` from `RenderingSystem`.
            *   Create a view for entities with `PositionComponent` and `StationComponent`.
            *   Iterate the view and call `IRenderer` methods to draw station shapes (circles, triangles, squares) at their positions.
            *   (Later) Add logic to visualize passenger count/overcrowding.
        4.  Register `StationRenderSystem` with `ECSCore`'s render phase.
        5.  Verify stations appear in the window.

**Objective 4: Implement Line Drawing (Player Interaction)**

*   **Task 4.1: Implement `LineDrawingSystem` (Input Handling)**
    *   Steps:
        1.  Define `LineDrawingSystem` interface/class based on `LineDrawingSystem.spec.md`.
        2.  Write integration tests (TDD style) simulating mouse clicks/drags on stations and verifying that `LineComponent` entities are created/modified correctly in the registry.
        3.  Implement `LineDrawingSystem` logic:
            *   Access `ECSCore` registry, `InputSystem`, and `RenderingSystem` (for coordinate mapping).
            *   In the update phase, check `InputSystem` for mouse clicks/drags.
            *   Convert screen coordinates to world coordinates.
            *   Identify if clicks are on existing `StationComponent` entities (requires spatial query or iteration).
            *   Handle state machine for drawing: Idle -> StartLine (click station) -> DraggingLine (drag to another station) -> EndLine (release on station).
            *   Create/update `LineComponent` entities based on interaction (add/remove stations from line). Assign a color.
        4.  Register `LineDrawingSystem` with `ECSCore`'s update phase.
        5.  Test drawing lines between stations interactively.

**Objective 5: Implement Line Rendering**

*   **Task 5.1: Implement `LineRenderSystem`**
    *   Steps:
        1.  Define `LineRenderSystem` interface/class based on `LineRenderSystem.spec.md`.
        2.  Write unit tests (TDD, mocking `IRenderer`) verifying that lines are drawn between the correct station positions based on `LineComponent` data.
        3.  Implement `LineRenderSystem` logic:
            *   Access `ECSCore` registry and `IRenderer`.
            *   Create a view for entities with `LineComponent`.
            *   For each line, get the list of station entities.
            *   For each station entity, get its `PositionComponent`.
            *   Call `IRenderer::drawLine` between consecutive station positions using the line's color.
        4.  Register `LineRenderSystem` with `ECSCore`'s render phase.
        5.  Verify drawn lines appear correctly.

**Objective 6: Implement Passenger Spawning**

*   **Task 6.1: Implement `PassengerGenerationSystem`**
    *   Steps:
        1.  Define `PassengerGenerationSystem` interface/class based on `PassengerGenerationSystem.spec.md`.
        2.  Write integration tests (TDD style) verifying that passenger entities are created periodically at stations with correct origin/destination information.
        3.  Implement `PassengerGenerationSystem` logic:
            *   Access `ECSCore` registry.
            *   Use a timer (based on `deltaTime`) to trigger spawning.
            *   Select a random origin `StationComponent` entity.
            *   Select a random destination station *type* (shape) different from the origin.
            *   Create a passenger entity with `PassengerComponent` (origin entity, destination type, state=waiting) and potentially `PositionComponent` (at origin station).
            *   Add the passenger entity reference to the origin station's passenger queue in its `StationComponent`.
        4.  Register `PassengerGenerationSystem` with `ECSCore`'s update phase.
        5.  (Debugging) Add logging or simple rendering to verify passengers spawn.

**Objective 7: Implement Vehicle Spawning & Assignment**

*   **Task 7.1: Add Vehicle Spawning to `LineDrawingSystem` (or new System)**
    *   Steps:
        1.  Modify `LineDrawingSystem` (or create `VehicleAssignmentSystem`) tests to verify vehicles are created when a line is completed.
        2.  Modify the system logic: When a `LineComponent` is first created (or modified significantly), create a `VehicleComponent` entity.
        3.  Associate the vehicle with the line (add vehicle entity to `LineComponent`, set line entity in `VehicleComponent`).
        4.  Initialize vehicle state (position at the first station on the line, target = second station).
        5.  Add `PositionComponent` to the vehicle entity.

**Objective 8: Implement Vehicle Movement**

*   **Task 8.1: Implement `VehicleMovementSystem`**
    *   Steps:
        1.  Define `VehicleMovementSystem` interface/class based on `VehicleMovementSystem.spec.md`.
        2.  Write unit tests (TDD) verifying vehicle position updates correctly towards the target station based on speed and `deltaTime`. Test arrival detection and target updating (moving to the next station on the line, wrapping around).
        3.  Implement `VehicleMovementSystem` logic:
            *   Access `ECSCore` registry.
            *   Create a view for entities with `VehicleComponent` and `PositionComponent`.
            *   For each vehicle:
                *   Get current position, target station entity, line entity, speed.
                *   Get target station's `PositionComponent`.
                *   Calculate direction vector towards the target.
                *   Update vehicle's `PositionComponent` based on direction, speed, and `deltaTime`.
                *   Check if the vehicle has reached/passed the target station.
                *   If reached:
                    *   Snap position to the station.
                    *   Update target to the *next* station on the `LineComponent`'s list (handle loop/reversal).
                    *   (Trigger passenger boarding/alighting logic - see next objective).
        4.  Register `VehicleMovementSystem` with `ECSCore`'s update phase.

**Objective 9: Implement Vehicle Rendering**

*   **Task 9.1: Implement `VehicleRenderSystem`**
    *   Steps:
        1.  Define `VehicleRenderSystem` interface/class based on `VehicleRenderSystem.spec.md`.
        2.  Write unit tests (TDD, mocking `IRenderer`) verifying vehicles are drawn at their correct positions.
        3.  Implement `VehicleRenderSystem` logic:
            *   Access `ECSCore` registry and `IRenderer`.
            *   Create a view for entities with `VehicleComponent` and `PositionComponent`.
            *   Iterate the view and call `IRenderer` methods (e.g., `drawRectangle`) to draw vehicles at their positions, potentially using the line's color.
        4.  Register `VehicleRenderSystem` with `ECSCore`'s render phase.
        5.  Verify vehicles appear and move along lines.

**Objective 10: Implement Passenger Boarding/Alighting**

*   **Task 10.1: Implement `PassengerBoardingSystem`**
    *   Steps:
        1.  Define `PassengerBoardingSystem` interface/class based on `PassengerBoardingSystem.spec.md`.
        2.  Write integration tests (TDD style) verifying:
            *   Passengers alight if the vehicle reaches their destination station type.
            *   Waiting passengers board if the vehicle goes towards their destination type and has capacity.
            *   Component data (`VehicleComponent` passenger list, `StationComponent` queue, `PassengerComponent` state) is updated correctly.
        3.  Implement `PassengerBoardingSystem` logic (likely triggered by `VehicleMovementSystem` upon arrival, or run after it):
            *   Access `ECSCore` registry.
            *   Identify vehicles that have just arrived at a station.
            *   **Alighting:** Iterate through passengers currently on the vehicle (`VehicleComponent::passengers`). If a passenger's destination type matches the current station's type, remove them from the vehicle list and destroy the passenger entity (or mark as arrived).
            *   **Boarding:** Iterate through passengers waiting at the station (`StationComponent::passengerQueue`). Check if the vehicle's line serves the passenger's destination type *and* the vehicle has capacity (`VehicleComponent::passengers.size() < capacity`). If so, move the passenger entity from the station queue to the vehicle list, and update the passenger's state (`PassengerComponent::state = Travelling`).
        4.  Register `PassengerBoardingSystem` with `ECSCore`'s update phase (ensure correct ordering relative to `VehicleMovementSystem`).
        5.  Test passenger transport flow.

**Objective 11: Implement Station Overcrowding (Game Over Condition)**

*   **Task 11.1: Implement `StationOvercrowdingSystem`**
    *   Steps:
        1.  Define `StationOvercrowdingSystem` interface/class based on `StationOvercrowdingSystem.spec.md`.
        2.  Write integration tests (TDD style) verifying that the game state changes (e.g., sets a 'GameOver' flag) when a station's passenger queue exceeds its capacity for a certain duration.
        3.  Implement `StationOvercrowdingSystem` logic:
            *   Access `ECSCore` registry.
            *   Create a view for entities with `StationComponent`.
            *   For each station, check if `passengerQueue.size()` exceeds `capacity`.
            *   If overcrowded, start or increment a timer associated with that station (could be stored in the component or a separate map/component).
            *   If the timer exceeds a threshold, trigger a game over state (e.g., set a flag in a global `GameState` component/resource, stop updates).
            *   If not overcrowded, reset the timer.
        4.  Register `StationOvercrowdingSystem` with `ECSCore`'s update phase.
        5.  Implement basic game over handling in `CoreApplication` (e.g., stop the main loop or display a message).