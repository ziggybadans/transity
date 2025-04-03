// File: specs/systems/StationRenderSystem.spec.md
// Module: Systems
// System: StationRenderSystem

# Specification: Station Render System

## 1. Overview

Responsible for drawing station entities onto the screen based on their type (shape) and position. May also visualize passenger counts or overcrowding status.

## 2. Dependencies

- `ECSCore`: To query entities with relevant components.
- `RenderingSystem`: To perform drawing operations (draw shapes, text) and access view information.
- `StationComponent`: To get station type, waiting passenger count, capacity, and identify station entities.
- `PositionComponent`: To get the world position for drawing.
- (Potentially) `ResourceManager`: To get textures or fonts if needed for more complex rendering (e.g., passenger count text).
- (Potentially) `ConfigSystem`: To get rendering settings (colors, sizes, shape definitions, font names/sizes).

## 3. Logic (Render Cycle - `render(renderer)`)

1.  **Get Station View:** Get a view of all entities with `StationComponent` and `PositionComponent`.
    - `registry.view<StationComponent, PositionComponent>()`
2.  **Per-Station Logic:** For each station entity (`stationEntity`, `stationComp`, `positionComp`):
    - **Check Visibility:** (Optional but Recommended) Check if `positionComp` is within the current view bounds provided by `RenderingSystem`. If not, skip rendering this station.
        - `TDD_ANCHOR: Test_StationRender_VisibilityCulling`
    - **Determine Shape & Style:**
        - Based on `stationComp.type` (CIRCLE, SQUARE, TRIANGLE, etc.), select the appropriate `sf::Shape` (e.g., `sf::CircleShape`, `sf::RectangleShape`). Triangles might require `sf::ConvexShape` or a `sf::VertexArray`.
        - Define shape properties: size/radius (from config/constant), fill color (e.g., white), outline color (e.g., black), outline thickness.
        - Check for overcrowding: `isOvercrowded = stationComp.waitingPassengers.size() >= stationComp.maxCapacity`. If true, potentially change outline color to red.
        - `TDD_ANCHOR: Test_StationRender_SelectShape_ByType`
        - `TDD_ANCHOR: Test_StationRender_SetShapeStyle_Default`
        - `TDD_ANCHOR: Test_StationRender_SetShapeStyle_Overcrowded`
    - **Set Position:** Set the shape's position using `positionComp.x`, `positionComp.y`. Adjust the shape's origin (`shape.setOrigin(...)`) so the position corresponds to the shape's center.
        - `TDD_ANCHOR: Test_StationRender_SetShapePositionAndOrigin`
    - **Draw Shape:** Call `RenderingSystem.draw(shape)`.
        - `TDD_ANCHOR: Test_StationRender_DrawShapeCall`
    - **Visualize Passengers (Optional):**
        - Get passenger count: `count = stationComp.waitingPassengers.size()`.
        - If `count > 0`:
            - **Option A (Text):**
                - Create/update an `sf::Text` object with the string representation of `count`.
                - Set font (loaded via `ResourceManager`), character size, fill color.
                - Set text position relative to the station shape (e.g., slightly offset). Center the text origin.
                - Call `RenderingSystem.draw(text)`.
                - `TDD_ANCHOR: Test_StationRender_DrawPassengerCount_Text`
            - **Option B (Indicators):**
                - Draw `count` small shapes (e.g., tiny squares) arranged near the station shape. This might involve calculating positions for each indicator.
                - `TDD_ANCHOR: Test_StationRender_DrawPassengerIndicators`

## 4. Edge Cases & Considerations

- **Shape Definitions:** Standardize sizes and visual styles for different station types, likely via configuration or constants.
- **Performance:** Drawing many individual shapes (especially passenger indicators) can impact performance. Using `sf::VertexArray` for indicators or batch rendering techniques could optimize this if needed.
- **Z-Ordering/Layering:** Ensure stations render correctly relative to lines and vehicles. This might involve setting Z coordinates or using distinct render passes/layers in the `RenderingSystem`.
- **Font Loading:** Text rendering requires fonts. A `ResourceManager` should handle loading and provide access to `sf::Font` objects.
- **Clutter:** Displaying many passenger indicators can become visually cluttered. Consider alternative visualizations (e.g., changing station color intensity, using only text above a certain count).

## 5. TDD Anchors Summary

- `Test_StationRender_VisibilityCulling`
- `Test_StationRender_SelectShape_ByType`
- `Test_StationRender_SetShapeStyle_Default`
- `Test_StationRender_SetShapeStyle_Overcrowded`
- `Test_StationRender_SetShapePositionAndOrigin`
- `Test_StationRender_DrawShapeCall`
- `Test_StationRender_DrawPassengerCount_Text`
- `Test_StationRender_DrawPassengerIndicators`