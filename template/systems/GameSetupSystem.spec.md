// File: specs/systems/GameSetupSystem.spec.md
// Module: Systems
// System: GameSetupSystem

# Specification: Game Setup System

## 1. Overview

Responsible for initializing the game world state when a new game starts. This includes creating the initial set of stations, potentially providing the player with starting resources (lines, vehicles), and setting the initial game state. This system typically runs only once at the beginning of a game session.

## 2. Dependencies

- `ECSCore`: To create entities and add components.
- `StationComponent`, `PositionComponent`, `LineComponent`, `VehicleComponent`, `OvercrowdingStateComponent`: Components to add to initial entities.
- `GameStateManager` / `GameEventSystem`: To set the game state to 'RUNNING' after setup.
- (Potentially) `ConfigSystem`: To get configuration for initial setup (number of stations, starting resources, map size/bounds).
- (Potentially) `RandomNumberGenerator`: To place initial stations randomly within bounds.
- (Potentially) `ResourceManager` / `ColorPalette`: To assign initial line colors.
- (Potentially) `PlayerResourceSystem`: To store/manage player's available lines/vehicles.

## 3. Logic (Run Once at Game Start)

1.  **Clear Existing State (If applicable):** If restarting a game, ensure the ECS registry is cleared of previous game entities (this might be handled by `CoreApplication` or `GameStateManager` before calling setup).
    - `TDD_ANCHOR: Test_Setup_ClearsPreviousState`
2.  **Load Configuration:** Get initial setup parameters from `ConfigSystem` (e.g., `initialStationCount`, `mapWidth`, `mapHeight`, `startingLineCount`, `startingVehicleCount`).
    - `TDD_ANCHOR: Test_Setup_LoadConfig`
3.  **Create Initial Stations:**
    - Loop `initialStationCount` times:
        - Choose a random, unoccupied position within map bounds (`x`, `y`) using `RandomNumberGenerator`. Ensure minimum distance from other stations.
            - `TDD_ANCHOR: Test_Setup_StationPosition_Random`
            - `TDD_ANCHOR: Test_Setup_StationPosition_WithinBounds`
            - `TDD_ANCHOR: Test_Setup_StationPosition_MinDistance`
        - Choose an initial `StationType` (e.g., cycle through CIRCLE, SQUARE, TRIANGLE). Ensure a mix of types if possible.
            - `TDD_ANCHOR: Test_Setup_StationType_Assignment`
            - `TDD_ANCHOR: Test_Setup_StationType_MixEnsured`
        - Create station entity: `stationEntity = ECSCore.createEntity()`.
        - Add `PositionComponent(x, y)` to `stationEntity`.
        - Add `StationComponent(type)` to `stationEntity`.
        - Add `OvercrowdingStateComponent` (if used) to `stationEntity`.
        - `TDD_ANCHOR: Test_Setup_CreateStationEntity_WithComponents`
4.  **Grant Initial Resources (MVP Simplification):**
    - **Lines:** Make `startingLineCount` line colors available to the player. This might involve:
        - Adding colors to a list in a `PlayerResourceSystem`.
        - Or pre-creating `startingLineCount` line entities with unique colors but no stations, ready to be used by `LineDrawingSystem`.
        - `TDD_ANCHOR: Test_Setup_GrantInitialLineColors`
    - **Vehicles:** Create `startingVehicleCount` vehicle entities.
        - Add `VehicleComponent` (initially unassigned: `assignedLine = entt::null`, state = `MOVING` or a specific `IDLE_IN_DEPOT` state).
        - Add `PositionComponent` (potentially off-screen or at a central point).
        - These vehicles would need to be assigned to lines by the player later (or automatically in a simpler MVP).
        - `TDD_ANCHOR: Test_Setup_CreateInitialVehicles`
        - `TDD_ANCHOR: Test_Setup_InitialVehicles_Unassigned`
5.  **Set Initial Game State:** Signal `GameStateManager` that setup is complete and the game state should transition to `RUNNING` (or `PLAYING`).
    - `TDD_ANCHOR: Test_Setup_SetGameState_Running`
6.  **Log Setup Completion:** Log successful game world initialization.

## 4. Edge Cases & Considerations

- **Map Bounds:** Define how map boundaries are determined and enforced for station placement.
- **Unoccupied Position:** Need a reliable way to check if a chosen random position is too close to existing stations. A simple distance check or a spatial partitioning structure could work. What if no valid position can be found after several tries?
- **Resource Representation:** How are available lines/vehicles tracked? A dedicated player resource component/system is likely needed for tracking usage and granting new resources over time.
- **Guaranteed Solvability:** Ensure the initial station types and layout allow for basic connections (e.g., at least two different station types are generated if `initialStationCount >= 2`).
- **Restarting Game:** Ensure this system correctly resets the state if called multiple times (e.g., via a "New Game" menu option). The clearing step is crucial.

## 5. TDD Anchors Summary

- `Test_Setup_ClearsPreviousState`
- `Test_Setup_LoadConfig`
- `Test_Setup_StationPosition_Random`
- `Test_Setup_StationPosition_WithinBounds`
- `Test_Setup_StationPosition_MinDistance`
- `Test_Setup_StationType_Assignment`
- `Test_Setup_StationType_MixEnsured`
- `Test_Setup_CreateStationEntity_WithComponents`
- `Test_Setup_GrantInitialLineColors`
- `Test_Setup_CreateInitialVehicles`
- `Test_Setup_InitialVehicles_Unassigned`
- `Test_Setup_SetGameState_Running`