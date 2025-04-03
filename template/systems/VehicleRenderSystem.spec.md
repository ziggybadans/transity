// File: specs/systems/VehicleRenderSystem.spec.md
// Module: Systems
// System: VehicleRenderSystem

# Specification: Vehicle Render System

## 1. Overview

Responsible for drawing vehicle entities onto the screen at their current position. The visual representation might be a simple shape initially, potentially colored based on the line the vehicle is assigned to. May also visualize passengers on board.

## 2. Dependencies

- `ECSCore`: To query entities with relevant components.
- `RenderingSystem`: To perform drawing operations.
- `VehicleComponent`: To identify vehicle entities and potentially get passenger count or assigned line for coloring.
- `PositionComponent`: To get the world position for drawing.
- `LineComponent`: (Optional) To get the color of the assigned line if vehicles should match line color.
- (Potentially) `ResourceManager`: To get vehicle sprites/textures if not using simple shapes.
- (Potentially) `ConfigSystem`: To get rendering settings (vehicle size, shape, default color).

## 3. Logic (Render Cycle - `render(renderer)`)

1.  **Get Vehicle View:** Get a view of all entities with `VehicleComponent` and `PositionComponent`.
    - `registry.view<VehicleComponent, PositionComponent>()`
2.  **Per-Vehicle Logic:** For each vehicle entity (`vehicleEntity`, `vehicleComp`, `positionComp`):
    - **Check Visibility:** (Optional but Recommended) Check if `positionComp` is within the current view bounds provided by `RenderingSystem`. If not, skip rendering.
        - `TDD_ANCHOR: Test_VehicleRender_VisibilityCulling`
    - **Determine Style & Shape:**
        - **Shape:** Define the vehicle shape (e.g., `sf::RectangleShape`). Get size from config/constants.
        - **Color:**
            - Option A: Use a default vehicle color (e.g., gray).
            - Option B: Get the `assignedLine` entity from `vehicleComp`. If valid, get the `LineComponent` for that line and use its `color` as the vehicle's fill color. Handle cases where the line is invalid or unassigned.
            - `TDD_ANCHOR: Test_VehicleRender_SetColor_Default`
            - `TDD_ANCHOR: Test_VehicleRender_SetColor_FromLine`
            - `TDD_ANCHOR: Test_VehicleRender_SetColor_HandleInvalidLine`
        - **Outline:** Set outline color (e.g., black) and thickness.
    - **Set Position & Origin:** Set the shape's position using `positionComp.x`, `positionComp.y`. Adjust origin for centering.
        - `TDD_ANCHOR: Test_VehicleRender_SetPositionAndOrigin`
    - **Draw Vehicle Shape:** Call `RenderingSystem.draw(vehicleShape)`.
        - `TDD_ANCHOR: Test_VehicleRender_DrawVehicleShape`
    - **Visualize Passengers (Optional):**
        - Get passenger count: `count = vehicleComp.passengers.size()`.
        - Get capacity: `capacity = vehicleComp.capacity`.
        - Draw small indicators (e.g., squares) inside or on top of the vehicle shape to represent passengers. The number of indicators could match `count`, or their color/fill could represent `count / capacity`.
        - `TDD_ANCHOR: Test_VehicleRender_DrawPassengerIndicators_OnVehicle`

## 4. Edge Cases & Considerations

- **Vehicle Appearance:** Simple rectangles are fine for MVP. Later phases might use sprites, requiring texture loading (`ResourceManager`) and potentially rotation based on movement direction.
- **Performance:** Drawing many passenger indicators per vehicle could be slow. Consider simpler visualizations (e.g., changing vehicle saturation based on load).
- **Z-Ordering:** Ensure vehicles render on top of lines but potentially underneath UI elements or station markers if desired.
- **Color Choice:** If using line color for vehicles, ensure sufficient contrast with the background and potentially the line itself (e.g., using the line color for the vehicle outline instead of fill).

## 5. TDD Anchors Summary

- `Test_VehicleRender_VisibilityCulling`
- `Test_VehicleRender_SetColor_Default`
- `Test_VehicleRender_SetColor_FromLine`
- `Test_VehicleRender_SetColor_HandleInvalidLine`
- `Test_VehicleRender_SetPositionAndOrigin`
- `Test_VehicleRender_DrawVehicleShape`
- `Test_VehicleRender_DrawPassengerIndicators_OnVehicle`