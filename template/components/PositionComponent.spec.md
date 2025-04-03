// File: specs/components/PositionComponent.spec.md
// Module: Components
// Component: PositionComponent

# Specification: Position Component

## 1. Overview

Stores the 2D position of an entity in the game world. This is a fundamental component used by entities that need to be located spatially (e.g., stations, vehicles, potentially UI elements in world space).

## 2. Dependencies

- (None directly, but relies on a coordinate system defined conceptually or by `RenderingSystem`'s view)

## 3. Data Structures

```cpp
struct PositionComponent {
    float x;
    float y;
    // Optional: float z; // If 2.5D or 3D elements are considered later
};
```

## 4. Data Members

- `x`: The X-coordinate in world space.
- `y`: The Y-coordinate in world space.

## 5. Usage

- Attached to entities that have a physical location in the game world.
- Read by `RenderingSystem` (or specific render systems) to determine where to draw the entity.
- Read and potentially modified by `MovementSystem` or other physics/simulation systems.
- Read by `InputSystem` (via coordinate mapping) to determine which entity is clicked/hovered.

## 6. Edge Cases & Considerations

- **Coordinate System:** Ensure consistency in the world coordinate system used (e.g., origin position, axis directions).
- **Precision:** `float` is likely sufficient for world coordinates unless extremely large worlds or high precision is needed, in which case `double` might be considered (with performance implications).
- **Z-Ordering:** For 2D rendering, if depth/layering is needed, a separate `LayerComponent` or using the Z value might be necessary.

## 7. TDD Anchors Summary

- `Test_Component_Position_Initialization`
- `Test_Component_Position_Update`
- `Test_System_Reads_Position` (Tested within systems using this component)