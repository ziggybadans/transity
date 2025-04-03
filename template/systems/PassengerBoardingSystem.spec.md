// File: specs/systems/PassengerBoardingSystem.spec.md
// Module: Systems
// System: PassengerBoardingSystem

# Specification: Passenger Boarding System

## 1. Overview

Manages the transfer of passenger entities between stations and vehicles when a vehicle is stopped at a station. Handles both passengers alighting (getting off) if they reached their destination type and passengers boarding (getting on) if the vehicle's line serves their desired destination type and the vehicle has capacity.

## 2. Dependencies

- `ECSCore`: To query and modify components.
- `VehicleComponent`: To check vehicle state (`STOPPED_AT_STATION`), capacity, current passengers, and assigned line.
- `StationComponent`: To access waiting passengers at the station and the station's type.
- `PassengerComponent`: To check passenger destination type.
- `LineComponent`: To check the sequence of stations on the vehicle's assigned line to determine if a passenger's destination is reachable.
- (Potentially) `LoggingSystem`: For logging boarding/alighting events.
- (Potentially) `GameEventSystem`: To signal passenger delivery for scoring/stats.

## 3. Logic (Update Cycle - `update(deltaTime)`)

1.  **Get Stopped Vehicle View:** Get a view of vehicle entities that are currently stopped at a station.
    - `registry.view<VehicleComponent>()` and filter for `vehicleComp.state == VehicleComponent::State::STOPPED_AT_STATION`.
2.  **Per Stopped Vehicle Logic:** For each stopped vehicle entity (`vehicleEntity`, `vehicleComp`):
    - **Get Current Station:**
        - Get the `LineComponent` (`lineComp`) for `vehicleComp.assignedLine`. Handle invalid line.
        - Get the current station entity (`currentStationEntity`) using `vehicleComp.currentSegmentIndex` from `lineComp.stations`. Handle invalid index/station.
        - Get the `StationComponent` (`stationComp`) for `currentStationEntity`. Handle invalid station component.
        - Get the `StationType` (`currentStationType`) from `stationComp.type`.
        - `TDD_ANCHOR: Test_Boarding_GetVehicleCurrentStation_Valid`
        - `TDD_ANCHOR: Test_Boarding_Handle_InvalidVehicleLineOrStation`
    - **Alighting Logic:**
        - Iterate through a copy of `vehicleComp.passengers` (to allow removal while iterating). For each `passengerEntity` on board:
            - Get `PassengerComponent` (`passengerComp`) for `passengerEntity`. Handle invalid passenger.
            - If `passengerComp.destinationType == currentStationType`:
                - Passenger reached destination.
                - Remove `passengerEntity` from `vehicleComp.passengers`.
                - Destroy `passengerEntity` using `ECSCore.destroyEntity()`.
                - Log alighting (e.g., "Passenger X alighted at Station Y").
                - (Optional) Trigger game event for successful delivery.
                - `TDD_ANCHOR: Test_Boarding_PassengerAlights_CorrectDestination`
                - `TDD_ANCHOR: Test_Boarding_PassengerRemovedFromVehicleOnAlight`
                - `TDD_ANCHOR: Test_Boarding_PassengerEntityDestroyedOnAlight`
            - Else:
                - Passenger stays on board.
                - `TDD_ANCHOR: Test_Boarding_PassengerStays_IncorrectDestination`
    - **Boarding Logic:**
        - Iterate through a copy of `stationComp.waitingPassengers` (to allow removal). For each `passengerEntity` waiting at the station:
            - **Check Vehicle Capacity:** If `vehicleComp.passengers.size() >= vehicleComp.capacity`, stop boarding attempts for this vehicle.
                - `TDD_ANCHOR: Test_Boarding_StopsAtVehicleCapacity`
                - Break the inner loop (no more room).
            - Get `PassengerComponent` (`passengerComp`) for `passengerEntity`. Handle invalid passenger.
            - **Check Destination Reachable:**
                - Determine if a station with `type == passengerComp.destinationType` exists on `lineComp.stations` *after* the `currentSegmentIndex` (considering line direction/looping).
                - `TDD_ANCHOR: Test_Boarding_CheckDestinationReachable_OnLine`
            - **If Reachable and Capacity Available:**
                - Remove `passengerEntity` from `stationComp.waitingPassengers`.
                - Add `passengerEntity` to `vehicleComp.passengers`.
                - Log boarding (e.g., "Passenger Y boarded Vehicle X at Station Z").
                - `TDD_ANCHOR: Test_Boarding_PassengerBoards_DestinationReachable`
                - `TDD_ANCHOR: Test_Boarding_PassengerRemovedFromStationOnBoard`
                - `TDD_ANCHOR: Test_Boarding_PassengerAddedToVehicleOnBoard`
            - Else (Not Reachable or No Capacity):
                - Passenger continues waiting at the station.
                - `TDD_ANCHOR: Test_Boarding_PassengerWaits_DestinationNotReachable`

## 4. Helper Functions

- `isDestinationOnLine(lineComp, currentStationIndex, destinationType)`: Internal helper function to check if a station of `destinationType` exists on the line after the `currentStationIndex`, respecting line direction/looping logic.

## 5. Edge Cases & Considerations

- **Simultaneous Boarding/Alighting:** The order matters slightly. Alighting first frees up space for potential boarders in the same update cycle.
- **Passenger Sorting:** Does the order passengers board/alight matter? (e.g., FIFO). For MVP, likely not critical.
- **Multiple Lines at Station:** In later phases, stations might serve multiple lines. Boarding logic needs to check if the *specific vehicle's line* serves the destination.
- **Transfer Logic:** Not needed for MVP, but later phases would require passengers to potentially alight and wait for another line.
- **Performance:** Checking reachability for every waiting passenger every time a vehicle stops could be slow if lines are long. Caching reachable types per line segment might optimize this. Iterating copies of vectors can also be inefficient; consider index-based iteration with careful removal.

## 6. TDD Anchors Summary

- `Test_Boarding_GetVehicleCurrentStation_Valid`
- `Test_Boarding_Handle_InvalidVehicleLineOrStation`
- `Test_Boarding_PassengerAlights_CorrectDestination`
- `Test_Boarding_PassengerRemovedFromVehicleOnAlight`
- `Test_Boarding_PassengerEntityDestroyedOnAlight`
- `Test_Boarding_PassengerStays_IncorrectDestination`
- `Test_Boarding_StopsAtVehicleCapacity`
- `Test_Boarding_CheckDestinationReachable_OnLine`
- `Test_Boarding_PassengerBoards_DestinationReachable`
- `Test_Boarding_PassengerRemovedFromStationOnBoard`
- `Test_Boarding_PassengerAddedToVehicleOnBoard`
- `Test_Boarding_PassengerWaits_DestinationNotReachable`