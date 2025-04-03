// File: specs/components/StationComponent.spec.md
// Module: Components
// Component: StationComponent

# Specification: Station Component

## 1. Overview

Identifies an entity as a transport station and holds data essential for its function within the Mini Metro MVP context, such as its shape/type (determining passenger destinations) and capacity.

## 2. Dependencies

- EnTT library (for `entt::entity`)
- (Potentially) A shared header defining `StationType` enum.

## 3. Data Structures

```cpp
#include <string>
#include <vector>
#include <entt/entt.hpp> // For entt::entity

// Enum definition (could be in a separate types.spec.md or similar)
enum class StationType {
    CIRCLE,
    SQUARE,
    TRIANGLE,
    // Add more types as needed (e.g., STAR, PENTAGON)
    INVALID
};

struct StationComponent {
    StationType type = StationType::INVALID; // The shape/type of this station
    int maxCapacity = 10;                    // Max waiting passengers before overcrowding
    std::vector<entt::entity> waitingPassengers; // Passenger entities currently at the station
    std::string name = "";                   // Optional: Display name
};
```

## 4. Data Members

- `type`: An enum value (`StationType`) indicating the shape/type of the station. In the MVP, passengers generated at this station will want to travel to stations of *different* types.
- `maxCapacity`: The maximum number of passengers allowed to wait at this station before triggering an overcrowded state (likely game over in the MVP). Defaulted to a reasonable value (e.g., 10).
- `waitingPassengers`: A dynamic list (e.g., `std::vector`) storing the `entt::entity` IDs of passenger entities currently waiting at this station.
- `name`: (Optional) A human-readable name for the station, potentially auto-generated or set later.

## 5. Usage

- Attached to entities representing stations.
- **PassengerGenerationSystem:** Reads `type` to determine valid destination types for new passengers. Adds newly created passenger entities to the `waitingPassengers` list.
- **PassengerBoardingSystem:** Reads `waitingPassengers` to identify potential passengers for boarding vehicles. Removes passengers from this list when they board.
- **StationOvercrowdingSystem:** Checks `waitingPassengers.size()` against `maxCapacity` to detect game over conditions.
- **Rendering System (via specific StationRenderSystem):** Reads `type` (along with `PositionComponent`) to draw the correct shape. May use `waitingPassengers.size()` to visualize crowd levels.
- **LineDrawingSystem:** Uses the presence of this component (along with `PositionComponent`) to identify entities that can be connected by lines.

## 6. Edge Cases & Considerations

- **Initialization:** Ensure stations are created with a valid `type` and appropriate `maxCapacity`.
- **Passenger List Management:** Systems modifying `waitingPassengers` must do so correctly to avoid double-counting, losing passengers, or incorrect capacity checks. Atomicity might be needed if updates could be concurrent (unlikely in basic loop).
- **Station Type Definition:** The `StationType` enum needs to be globally accessible and potentially extensible later.
- **Capacity Zero:** What happens if `maxCapacity` is set to 0 or less? Should probably default to a minimum positive value.

## 7. TDD Anchors Summary

- `Test_Component_Station_Initialization` (Check default type, capacity)
- `Test_Component_Station_AddWaitingPassenger`
- `Test_Component_Station_RemoveWaitingPassenger`
- `Test_Component_Station_PassengerCountMatchesVectorSize`
- `Test_System_Reads_StationType` (Tested within relevant systems)
- `Test_System_Reads_StationCapacity` (Tested within StationOvercrowdingSystem)
- `Test_System_Modifies_WaitingPassengers` (Tested within PassengerGeneration, PassengerBoarding)