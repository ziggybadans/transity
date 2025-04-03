// File: specs/systems/VehicleMovementSystem.spec.md
// Module: Systems
// System: VehicleMovementSystem

# Specification: Vehicle Movement System

## 1. Overview

This system updates the position and state of vehicles based on the lines they are assigned to, their speed, and their interactions with stations (arrival, dwelling).

## 2. Dependencies

- `ECSCore`: To query entities and components.
- `VehicleComponent`: To get vehicle state (assigned line, progress, speed, state, dwell timer).
- `LineComponent`: To get the sequence of stations for the assigned line.
- `StationComponent`: To get station data (needed for position).
- `PositionComponent`: To read station positions and update vehicle positions.
- `GameClock` / `deltaTime`: To perform time-based movement calculations.
- (Potentially) `LoggingSystem`: For debugging movement issues.

## 3. Logic (Update Cycle - `update(deltaTime)`)

1.  **Get Vehicle View:** Get a view of all entities with `VehicleComponent` and `PositionComponent`.
    - `registry.view<VehicleComponent, PositionComponent>()`
2.  **Per-Vehicle Logic:** For each vehicle entity (`vehicleEntity`, `vehicleComp`, `vehiclePos`):
    - **Check Assignment:** If `vehicleComp.assignedLine == entt::null`, skip this vehicle (it's not on a line).
        - `TDD_ANCHOR: Test_VehicleMove_Skip_Unassigned`
    - **Get Line and Station Data:**
        - Try to get the `LineComponent` (`lineComp`) for `vehicleComp.assignedLine`. If invalid (line deleted?), handle appropriately (e.g., remove vehicle, log error, skip).
            - `TDD_ANCHOR: Test_VehicleMove_Handle_InvalidLine`
        - Get the list of station entities: `stations = lineComp.stations`.
        - If `stations.size() < 2`, the line is invalid for movement. Handle appropriately (e.g., stop vehicle, log error, skip).
            - `TDD_ANCHOR: Test_VehicleMove_Handle_LineTooShort`
    - **Handle State:**
        - **If `vehicleComp.state == VehicleComponent::State::STOPPED_AT_STATION`:**
            - Decrement `vehicleComp.dwellTimer` by `deltaTime`.
            - If `vehicleComp.dwellTimer <= 0`:
                - Set `vehicleComp.state = VehicleComponent::State::MOVING`.
                - Determine the *next* station index (`nextStationIndex`) based on current `currentSegmentIndex` and line direction logic (e.g., increment, wrap around, reverse direction).
                - Update `vehicleComp.currentSegmentIndex = nextStationIndex`.
                - Reset `vehicleComp.progressOnSegment = 0.0f`.
                - Log departure (e.g., "Vehicle X departing Station Y towards Station Z").
                - `TDD_ANCHOR: Test_VehicleMove_DwellTimer_Decrement`
                - `TDD_ANCHOR: Test_VehicleMove_DwellTimer_Expires_StateChange`
                - `TDD_ANCHOR: Test_VehicleMove_DwellTimer_Expires_IndexUpdate`
        - **If `vehicleComp.state == VehicleComponent::State::MOVING`:**
            - **Get Segment Stations:**
                - Determine the index of the station the vehicle is coming *from* (`fromStationIndex`). This depends on `currentSegmentIndex` and direction logic (e.g., `currentSegmentIndex - 1`, handling wrap-around).
                - Get the entity IDs for the 'from' and 'to' stations: `fromStationEntity = stations[fromStationIndex]`, `toStationEntity = stations[vehicleComp.currentSegmentIndex]`.
                - Try to get `PositionComponent` for both stations (`fromPos`, `toPos`). Handle errors if stations are invalid.
                - `TDD_ANCHOR: Test_VehicleMove_GetSegmentStations_Valid`
                - `TDD_ANCHOR: Test_VehicleMove_Handle_InvalidStationInLine`
            - **Calculate Movement:**
                - Calculate the vector for the current segment: `segmentVector = toPos - fromPos`.
                - Calculate the length of the segment: `segmentLength = length(segmentVector)`. Handle zero length segments (log warning, potentially skip movement).
                    - `TDD_ANCHOR: Test_VehicleMove_CalculateSegmentLength`
                    - `TDD_ANCHOR: Test_VehicleMove_Handle_ZeroLengthSegment`
                - Calculate distance to move this frame: `deltaDistance = vehicleComp.speed * deltaTime`.
                - Calculate progress increment for this frame: `deltaProgress = deltaDistance / segmentLength` (if `segmentLength > 0`).
            - **Update Progress:**
                - `vehicleComp.progressOnSegment += deltaProgress`.
                - `TDD_ANCHOR: Test_VehicleMove_ProgressUpdate`
            - **Check Arrival:**
                - If `vehicleComp.progressOnSegment >= 1.0`:
                    - Arrived at `toStationEntity`.
                    - Set vehicle's `vehiclePos` exactly to `toPos`.
                    - Set `vehicleComp.state = VehicleComponent::State::STOPPED_AT_STATION`.
                    - Set `vehicleComp.dwellTimer` to a default dwell time (e.g., from config or constant).
                    - Keep `currentSegmentIndex` pointing to the station just arrived at (it becomes the target for the *next* segment after dwelling).
                    - Log arrival (e.g., "Vehicle X arrived at Station Y").
                    - `TDD_ANCHOR: Test_VehicleMove_Arrival_StateChange`
                    - `TDD_ANCHOR: Test_VehicleMove_Arrival_PositionSnap`
                    - `TDD_ANCHOR: Test_VehicleMove_Arrival_DwellTimerSet`
                - **Else (Still Moving):**
                    - Interpolate position: `vehiclePos = fromPos + segmentVector * vehicleComp.progressOnSegment`.
                    - `TDD_ANCHOR: Test_VehicleMove_InterpolatePosition`

## 4. Edge Cases & Considerations

- **Line Direction:** Define clearly how vehicles navigate lines (e.g., loop, back-and-forth). The logic for determining `fromStationIndex` and `nextStationIndex` depends heavily on this.
- **Variable Speed:** Vehicle speed might change based on track type, congestion, etc. in later phases.
- **Segment Length & Speed:** Ensure `progressOnSegment` calculation correctly handles varying segment lengths relative to speed and `deltaTime`. Very short segments or high speeds might cause overshoot if not handled carefully (clamping progress or handling multiple segment traversals per update).
- **Floating Point Precision:** Be mindful of potential precision issues with `progressOnSegment`, especially comparing >= 1.0. Use a small epsilon or ensure calculations are robust.
- **Performance:** Getting components repeatedly in the loop can be slow. EnTT views are efficient, but accessing components of related entities (stations on the line) requires care. Caching station positions might be an option if they don't move.

## 5. TDD Anchors Summary

- `Test_VehicleMove_Skip_Unassigned`
- `Test_VehicleMove_Handle_InvalidLine`
- `Test_VehicleMove_Handle_LineTooShort`
- `Test_VehicleMove_DwellTimer_Decrement`
- `Test_VehicleMove_DwellTimer_Expires_StateChange`
- `Test_VehicleMove_DwellTimer_Expires_IndexUpdate`
- `Test_VehicleMove_GetSegmentStations_Valid`
- `Test_VehicleMove_Handle_InvalidStationInLine`
- `Test_VehicleMove_CalculateSegmentLength`
- `Test_VehicleMove_Handle_ZeroLengthSegment`
- `Test_VehicleMove_ProgressUpdate`
- `Test_VehicleMove_Arrival_StateChange`
- `Test_VehicleMove_Arrival_PositionSnap`
- `Test_VehicleMove_Arrival_DwellTimerSet`
- `Test_VehicleMove_InterpolatePosition`