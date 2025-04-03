// File: specs/components/LineComponent.spec.md
// Module: Components
// Component: LineComponent

# Specification: Line Component

## 1. Overview

Represents a single transport line connecting a sequence of stations. It holds the ordered list of station entities served by the line and visual properties like its color. Vehicles will be assigned to operate on these lines.

## 2. Dependencies

- EnTT library (for `entt::entity`)
- SFML library (for `sf::Color`)
- `StationComponent`: Lines connect entities that have a `StationComponent`.

## 3. Data Structures

```cpp
#include <vector>
#include <entt/entt.hpp> // For entt::entity
#include <SFML/Graphics/Color.hpp> // For sf::Color

struct LineComponent {
    std::vector<entt::entity> stations; // Ordered list of station entities on this line
    sf::Color color = sf::Color::White; // Visual color of the line
    std::string name = "";              // Optional: Name of the line (e.g., "Red Line")
    // Optional: LineType type; // Could differentiate between metro, bus later
};
```

## 4. Data Members

- `stations`: An ordered `std::vector` of `entt::entity` IDs representing the stations served by this line, in sequence. The order dictates the path vehicles will follow.
- `color`: An `sf::Color` value used for rendering the line on the map.
- `name`: (Optional) A human-readable name for the line.

## 5. Usage

- Attached to entities representing transport lines (these might be abstract entities without a direct visual representation other than the drawn line itself).
- **LineDrawingSystem:** Creates entities with `LineComponent` when the player draws a connection between stations. Modifies the `stations` vector as the player extends or modifies the line. Assigns a unique `color`.
- **VehicleAssignmentSystem:** Assigns vehicle entities to operate on a specific line entity (perhaps by adding a `VehicleComponent` with a reference to the line entity).
- **VehicleMovementSystem:** Reads the `stations` list from the line the vehicle is assigned to, determining the path the vehicle should follow between station positions.
- **PassengerBoardingSystem:** Reads the `stations` list to determine if a passenger's `destinationType` exists on this line, influencing boarding decisions.
- **Rendering System (via specific LineRenderSystem):** Reads the `stations` list (and their `PositionComponent`s) and the `color` to draw the line segments connecting the stations on the map.

## 6. Edge Cases & Considerations

- **Minimum Stations:** A line needs at least two stations to be functional. How are lines with 0 or 1 station handled? (Should likely be prevented by `LineDrawingSystem` or automatically deleted).
- **Duplicate Stations:** Should the `stations` list allow the same station entity multiple times consecutively or non-consecutively? (Mini Metro allows loops, so non-consecutive duplicates are okay).
- **Station Validity:** What happens if an entity ID in the `stations` list no longer refers to a valid entity with a `StationComponent` (e.g., station was deleted)? Systems using the list need to check validity.
- **Line Modification:** How are modifications (adding/removing stations mid-line, splitting lines) handled? The `LineDrawingSystem` needs robust logic for this.
- **Color Uniqueness:** Ensure assigned line colors are visually distinct, potentially cycling through a predefined palette.

## 7. TDD Anchors Summary

- `Test_Component_Line_Initialization`
- `Test_Component_Line_AddStation`
- `Test_Component_Line_RemoveStation`
- `Test_Component_Line_GetStations`
- `Test_Component_Line_SetColor`
- `Test_System_Creates_LineComponent` (Tested in LineDrawingSystem)
- `Test_System_Reads_LineStations` (Tested in VehicleMovement, PassengerBoarding, LineRenderSystem)
- `Test_System_Reads_LineColor` (Tested in LineRenderSystem)