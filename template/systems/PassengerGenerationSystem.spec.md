// File: specs/systems/PassengerGenerationSystem.spec.md
// Module: Systems
// System: PassengerGenerationSystem

# Specification: Passenger Generation System

## 1. Overview

This system is responsible for periodically spawning new passenger entities at existing stations. The type of passenger generated (i.e., their desired destination type) depends on the types of other stations available on the map.

## 2. Dependencies

- `ECSCore`: To create passenger entities and query station entities/components.
- `StationComponent`: To identify stations, access their type, and add waiting passengers.
- `PassengerComponent`: To add to the new passenger entities and set their destination type.
- `PositionComponent`: To potentially place the passenger visually near the station (though initially, they might just exist logically in the station's list).
- (Potentially) `GameClock` or `TimerSystem`: To control the rate of passenger spawning.
- (Potentially) `ConfigSystem`: To get configuration values like spawn rate, base spawn chance.
- (Potentially) `RandomNumberGenerator`: To introduce randomness in spawning times and destination selection.

## 3. Logic (Update Cycle)

1.  **Check Spawn Timer:** Determine if enough time has passed since the last spawn attempt (using `GameClock` / `TimerSystem` or a simple internal timer).
    - `TDD_ANCHOR: Test_PassengerGen_Timer_Check`
2.  **Iterate Stations:** If it's time to potentially spawn, get a view of all entities with `StationComponent`.
    - `registry.view<StationComponent>()`
3.  **Per-Station Logic:** For each station entity:
    - **Calculate Spawn Chance:** Determine the probability of a passenger spawning at this station during this update cycle. This could be a base chance modified by factors like time, station type, current passenger load (maybe later).
        - `TDD_ANCHOR: Test_PassengerGen_SpawnChance_Calculation`
    - **Roll for Spawn:** Use `RandomNumberGenerator` to see if a spawn occurs based on the calculated chance.
        - `TDD_ANCHOR: Test_PassengerGen_Spawn_Roll`
    - **If Spawn Occurs:**
        - **Determine Destination Type:**
            - Get the type (`originType`) of the current station from its `StationComponent`.
            - Find all *other* available `StationType`s currently present on the map (by iterating the station view again or maintaining a cached list of active types).
            - If other types exist, randomly select one (`destinationType`) that is *different* from `originType`.
            - If no other types exist, cannot spawn this type of passenger (skip or handle differently).
            - `TDD_ANCHOR: Test_PassengerGen_FindAvailableDestinationTypes`
            - `TDD_ANCHOR: Test_PassengerGen_SelectDifferentDestinationType`
            - `TDD_ANCHOR: Test_PassengerGen_HandleNoOtherTypes`
        - **Check Station Capacity:** Check if the current station's `waitingPassengers.size()` is less than its `maxCapacity`.
            - `TDD_ANCHOR: Test_PassengerGen_CheckStationCapacity_BelowLimit`
        - **If Capacity Available:**
            - **Create Passenger Entity:** `passengerEntity = ECSCore.createEntity()`
            - **Add PassengerComponent:** `ECSCore.addComponent<PassengerComponent>(passengerEntity, destinationType)`
            - **Add PositionComponent (Optional but Recommended):** Add `PositionComponent` to the passenger, initially setting its position equal to the station's position. `ECSCore.addComponent<PositionComponent>(passengerEntity, stationPosition)`
            - **Add to Station List:** Add `passengerEntity` to the `waitingPassengers` vector in the station's `StationComponent`.
            - Log passenger creation (e.g., "Passenger spawned at Station X wanting Type Y").
            - `TDD_ANCHOR: Test_PassengerGen_PassengerEntity_Created`
            - `TDD_ANCHOR: Test_PassengerGen_PassengerComponent_AddedWithDestination`
            - `TDD_ANCHOR: Test_PassengerGen_AddedToStationWaitingList`
        - **Else (Capacity Full):**
            - Log failure (e.g., "Spawn failed at Station X: capacity full").
            - `TDD_ANCHOR: Test_PassengerGen_CheckStationCapacity_AtLimit`
4.  **Reset Timer:** Reset or update the spawn timer for the next cycle.

## 4. Edge Cases & Considerations

- **Spawn Rate:** How is the overall spawn rate controlled? Global timer? Per-station timers? Needs configuration.
- **Initial Spawn:** How are the first passengers generated when the game starts?
- **No Other Station Types:** If only one type of station exists, passengers cannot be generated according to the standard rule. How is this handled? (No spawns? Special passenger type?)
- **Performance:** Iterating all stations frequently might be costly on large maps. Caching active station types could help.
- **Randomness:** Ensure the random number generator is seeded properly for replayability or variation as desired.
- **Difficulty Scaling:** Spawn rate and chance should likely increase over time to increase difficulty.

## 5. TDD Anchors Summary

- `Test_PassengerGen_Timer_Check`
- `Test_PassengerGen_SpawnChance_Calculation`
- `Test_PassengerGen_Spawn_Roll`
- `Test_PassengerGen_FindAvailableDestinationTypes`
- `Test_PassengerGen_SelectDifferentDestinationType`
- `Test_PassengerGen_HandleNoOtherTypes`
- `Test_PassengerGen_CheckStationCapacity_BelowLimit`
- `Test_PassengerGen_PassengerEntity_Created`
- `Test_PassengerGen_PassengerComponent_AddedWithDestination`
- `Test_PassengerGen_AddedToStationWaitingList`
- `Test_PassengerGen_CheckStationCapacity_AtLimit`