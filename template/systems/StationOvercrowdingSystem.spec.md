// File: specs/systems/StationOvercrowdingSystem.spec.md
// Module: Systems
// System: StationOvercrowdingSystem

# Specification: Station Overcrowding System

## 1. Overview

Monitors the number of waiting passengers at each station and compares it against the station's capacity. If a station remains overcrowded for a certain duration, this system triggers a game over state.

## 2. Dependencies

- `ECSCore`: To query station entities and components.
- `StationComponent`: To get waiting passenger count (`waitingPassengers.size()`) and `maxCapacity`.
- `GameClock` / `deltaTime`: To track how long a station has been overcrowded.
- `GameStateManager` / `GameEventSystem`: To signal the game over event/state change.
- (Potentially) `ConfigSystem`: To get the duration threshold for overcrowding before game over.

## 3. Data Structures

- **Component (Add to Station Entity):**
    ```cpp
    struct OvercrowdingStateComponent {
        bool isOvercrowded = false;
        float timeOvercrowded = 0.0f;
    };
    ```
    *(Alternatively, this state could be managed internally within the system using a map `[entt::entity -> float]`)*

- **Configuration:**
    - `OVERCROWDING_DURATION_THRESHOLD`: float (e.g., 5.0 seconds) - Time a station must be overcrowded before game over.

## 4. Logic (Update Cycle - `update(deltaTime)`)

1.  **Get Station View:** Get a view of all entities with `StationComponent` and `OvercrowdingStateComponent` (or manage state internally).
    - `registry.view<StationComponent, OvercrowdingStateComponent>()`
2.  **Per-Station Logic:** For each station entity (`stationEntity`, `stationComp`, `overcrowdingState`):
    - **Check Current Load:** Compare `stationComp.waitingPassengers.size()` with `stationComp.maxCapacity`.
    - **If `waitingPassengers.size() >= maxCapacity`:**
        - Station is currently overcrowded.
        - If `overcrowdingState.isOvercrowded == false`:
            - Mark it as overcrowded: `overcrowdingState.isOvercrowded = true`.
            - Reset timer: `overcrowdingState.timeOvercrowded = 0.0f`.
            - Log event (e.g., "Station X is now overcrowded!").
            - `TDD_ANCHOR: Test_Overcrowd_Detect_StartsTimer`
        - Increment timer: `overcrowdingState.timeOvercrowded += deltaTime`.
        - `TDD_ANCHOR: Test_Overcrowd_Timer_Increments`
        - **Check Threshold:** If `overcrowdingState.timeOvercrowded >= OVERCROWDING_DURATION_THRESHOLD`:
            - Trigger Game Over: Signal the `GameStateManager` or fire a `GameOverEvent`.
            - Log game over reason (e.g., "Game Over: Station X overcrowded for too long.").
            - (The system might stop further checks once game over is triggered).
            - `TDD_ANCHOR: Test_Overcrowd_ThresholdReached_TriggersGameOver`
    - **Else (`waitingPassengers.size() < maxCapacity`):**
        - Station is not currently overcrowded.
        - If `overcrowdingState.isOvercrowded == true`:
            - Mark it as not overcrowded: `overcrowdingState.isOvercrowded = false`.
            - Reset timer: `overcrowdingState.timeOvercrowded = 0.0f`.
            - Log event (e.g., "Station X is no longer overcrowded.").
            - `TDD_ANCHOR: Test_Overcrowd_Recover_ResetsState`
        - Timer remains 0.

## 5. Edge Cases & Considerations

- **State Management:** Decide whether to use an `OvercrowdingStateComponent` or manage the state internally within the system. A component is more ECS-idiomatic but adds component overhead. Internal state might be simpler for just this system.
- **Game Over Handling:** How is the game over state actually handled? Does this system set a global flag? Does it send an event? This depends on the design of the `GameStateManager`.
- **Visual Feedback:** While this system detects overcrowding, a separate system (e.g., `StationRenderSystem`) should provide visual feedback (like the station flashing red). This system might just set a flag/component state that the render system reads.
- **Threshold Configuration:** The `OVERCROWDING_DURATION_THRESHOLD` should be configurable.
- **Fluctuations:** Ensure brief moments of overcrowding don't trigger game over instantly; the timer handles this.

## 6. TDD Anchors Summary

- `Test_Overcrowd_Detect_StartsTimer`
- `Test_Overcrowd_Timer_Increments`
- `Test_Overcrowd_ThresholdReached_TriggersGameOver`
- `Test_Overcrowd_Recover_ResetsState`
- `Test_Overcrowd_Handles_FluctuationsNearCapacity`