// File: specs/components/PassengerComponent.spec.md
// Module: Components
// Component: PassengerComponent

# Specification: Passenger Component

## 1. Overview

Identifies an entity as a passenger and stores information about their travel desire, specifically the type (shape) of station they want to reach.

## 2. Dependencies

- (Potentially) A shared header defining `StationType` enum (same as used by `StationComponent`).

## 3. Data Structures

```cpp
// Assumes StationType enum is defined elsewhere (e.g., in StationComponent.spec.md or types.spec.md)
// enum class StationType { CIRCLE, SQUARE, TRIANGLE, ..., INVALID };

struct PassengerComponent {
    StationType destinationType = StationType::INVALID; // The type of station the passenger wants to go to
    // Optional: float patienceTimer = 60.0f; // Time before passenger gets unhappy? (Maybe later phase)
    // Optional: entt::entity currentStation; // Track where they are waiting? (Could be inferred from StationComponent::waitingPassengers)
};

```

## 4. Data Members

- `destinationType`: An enum value (`StationType`) indicating the shape/type of station the passenger wishes to travel to. This must be different from the type of the station where they spawn.

## 5. Usage

- Attached to entities representing passengers.
- **PassengerGenerationSystem:** Sets the `destinationType` when creating a new passenger entity, ensuring it's a valid type different from the origin station's type.
- **PassengerBoardingSystem:** Reads `destinationType` to determine if a waiting passenger should board a vehicle arriving at their current station, based on whether the vehicle's line includes a station of the desired type.
- **Rendering System (via specific PassengerRenderSystem):** Reads `destinationType` (along with `PositionComponent`, potentially inferred from the station they are at or the vehicle they are in) to draw the passenger icon (e.g., a small shape matching the destination type).

## 6. Edge Cases & Considerations

- **Invalid Destination:** Ensure `destinationType` is always set to a valid `StationType` upon creation.
- **No Route Available:** How does the simulation handle passengers whose `destinationType` is unreachable via the current network? (In Mini Metro, they just wait and contribute to overcrowding).
- **Passenger State:** For the MVP, passengers are simple (spawn -> wait -> board -> travel -> despawn). Later phases will require more state (e.g., `WAITING`, `TRAVELLING`, `ARRIVED`). This component might expand or be supplemented.

## 7. TDD Anchors Summary

- `Test_Component_Passenger_Initialization` (Check default destination type)
- `Test_Component_Passenger_SetDestination`
- `Test_System_Reads_PassengerDestination` (Tested within relevant systems like PassengerBoarding)
- `Test_System_Sets_PassengerDestination` (Tested within PassengerGeneration)