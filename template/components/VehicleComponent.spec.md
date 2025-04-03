// File: specs/components/VehicleComponent.spec.md
// Module: Components
// Component: VehicleComponent

# Specification: Vehicle Component

## 1. Overview

Identifies an entity as a vehicle (e.g., train, bus) and stores its operational state, including the line it's assigned to, its progress along that line, passenger capacity, and currently boarded passengers.

## 2. Dependencies

- EnTT library (for `entt::entity`)
- `LineComponent`: Vehicles operate on lines.
- `PassengerComponent`: Vehicles transport passengers.
- `PositionComponent`: Although position might be calculated by the movement system based on line progress, the vehicle entity itself still needs a `PositionComponent` for rendering and interaction.

## 3. Data Structures

```cpp
#include <vector>
#include <entt/entt.hpp> // For entt::entity

struct VehicleComponent {
    entt::entity assignedLine = entt::null; // The Line entity this vehicle operates on
    int currentSegmentIndex = 0;          // Index of the station in LineComponent::stations the vehicle is heading towards
    float progressOnSegment = 0.0f;       // Value from 0.0 to 1.0 indicating progress between the previous station and currentSegmentIndex station
    int capacity = 6;                     // Max passengers the vehicle can hold
    std::vector<entt::entity> passengers; // Passenger entities currently on board
    float speed = 50.0f;                  // Units per second (or per update tick, TBD)
    enum class State { MOVING, STOPPED_AT_STATION } state = State::MOVING; // Current state
    float dwellTimer = 0.0f;              // Time remaining stopped at a station
};
```

## 4. Data Members

- `assignedLine`: The `entt::entity` ID of the line this vehicle is assigned to. `entt::null` if unassigned.
- `currentSegmentIndex`: The index within the `assignedLine`'s `stations` vector indicating the *next* station the vehicle is moving towards. If index is 0, it might be moving from the last station back to the first (or vice-versa depending on direction logic).
- `progressOnSegment`: A normalized value (0.0 to 1.0) indicating how far the vehicle has travelled along the current line segment (between station `currentSegmentIndex - 1` and station `currentSegmentIndex`, handling wrap-around).
- `capacity`: The maximum number of passengers this vehicle can carry.
- `passengers`: A `std::vector` of `entt::entity` IDs for the passengers currently on board.
- `speed`: The movement speed of the vehicle along the track (units per second/tick).
- `state`: Enum indicating if the vehicle is currently `MOVING` or `STOPPED_AT_STATION`.
- `dwellTimer`: Countdown timer for how long the vehicle remains stopped at a station.

## 5. Usage

- Attached to entities representing vehicles.
- **VehicleAssignmentSystem:** Sets the `assignedLine` when a vehicle is added to a line.
- **VehicleMovementSystem:**
    - Reads `assignedLine`, `currentSegmentIndex`, `progressOnSegment`, `speed`, and `state`.
    - Reads `PositionComponent` of the current and next stations from the `LineComponent`.
    - Updates `progressOnSegment` based on `speed` and `deltaTime`.
    - When `progressOnSegment` >= 1.0:
        - Updates `currentSegmentIndex` (handling line ends/loops).
        - Resets `progressOnSegment`.
        - Sets `state` to `STOPPED_AT_STATION` and initializes `dwellTimer`.
        - Updates the vehicle's own `PositionComponent` to match the station's position.
    - While `MOVING`, interpolates the vehicle's `PositionComponent` between the previous and next station based on `progressOnSegment`.
    - Decrements `dwellTimer` when `STOPPED_AT_STATION`. When timer reaches zero, sets `state` back to `MOVING`.
- **PassengerBoardingSystem:**
    - Operates when `state` is `STOPPED_AT_STATION`.
    - Reads `capacity` and `passengers` list.
    - Allows passengers from the station (`StationComponent::waitingPassengers`) to board if their `destinationType` is served by the `assignedLine` *after* the current station, and if `passengers.size() < capacity`. Moves passenger entity ID from station list to vehicle list.
    - Allows passengers on board (`passengers` list) to alight if the current station's `type` matches their `destinationType`. Removes passenger entity ID from vehicle list and potentially destroys the passenger entity.
- **Rendering System (via specific VehicleRenderSystem):** Reads the vehicle's `PositionComponent` (updated by `VehicleMovementSystem`) to draw the vehicle sprite/shape. May also use `assignedLine`'s color.

## 6. Edge Cases & Considerations

- **Line Modification:** What happens if the `assignedLine` is modified (stations added/removed) while the vehicle is on it? The `VehicleMovementSystem` needs to handle potential invalid `currentSegmentIndex` or changes in the path. (Simplest MVP might ignore mid-journey modifications).
- **Line Deletion:** What happens if the `assignedLine` entity is destroyed? The vehicle should probably be removed or become inactive.
- **Capacity Full:** Ensure `PassengerBoardingSystem` respects vehicle `capacity`.
- **Segment Length:** Movement speed might need to be adjusted based on the actual distance between stations if segments have varying lengths, or the `progressOnSegment` calculation needs to account for distance.
- **Direction:** How is vehicle direction determined on the line (always forward, back-and-forth)? `VehicleMovementSystem` needs clear logic for updating `currentSegmentIndex` at line ends.
- **Initialization:** Ensure vehicles are created with a valid state, potentially linked to a line immediately or placed in a depot initially.

## 7. TDD Anchors Summary

- `Test_Component_Vehicle_Initialization`
- `Test_Component_Vehicle_AssignLine`
- `Test_Component_Vehicle_AddPassenger`
- `Test_Component_Vehicle_RemovePassenger`
- `Test_Component_Vehicle_PassengerCount`
- `Test_Component_Vehicle_IsAtFullCapacity`
- `Test_System_MovesVehicle_AlongSegment` (Tested in VehicleMovementSystem)
- `Test_System_MovesVehicle_ReachesStation` (Tested in VehicleMovementSystem)
- `Test_System_MovesVehicle_HandlesLineEnd` (Tested in VehicleMovementSystem)
- `Test_System_Vehicle_DwellTime` (Tested in VehicleMovementSystem)
- `Test_System_PassengerBoarding_VehicleCapacity` (Tested in PassengerBoardingSystem)
- `Test_System_PassengerAlighting_CorrectStation` (Tested in PassengerBoardingSystem)
- `Test_System_Reads_VehiclePosition` (Tested in RenderingSystem)