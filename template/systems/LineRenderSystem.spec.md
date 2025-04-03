// File: specs/systems/LineRenderSystem.spec.md
// Module: Systems
// System: LineRenderSystem

# Specification: Line Render System

## 1. Overview

Draws the transport lines on the screen by connecting the positions of the stations associated with each line entity, using the color specified in the `LineComponent`. Also handles drawing the temporary preview line during line creation/modification.

## 2. Dependencies

- `ECSCore`: To query entities and components.
- `RenderingSystem`: To perform drawing operations (draw vertex arrays/shapes).
- `LineComponent`: To get the list of stations and the line color.
- `StationComponent`: To validate station entities.
- `PositionComponent`: To get the world positions of stations.
- `LineDrawingSystem`: To check if a preview line needs to be drawn and get its points.
- (Potentially) `ConfigSystem`: To get line thickness settings.

## 3. Logic (Render Cycle - `render(renderer)`)

1.  **Draw Existing Lines:**
    - Get a view of all entities with `LineComponent`.
        - `registry.view<LineComponent>()`
    - **Per-Line Logic:** For each line entity (`lineEntity`, `lineComp`):
        - Get the list of station entities: `stations = lineComp.stations`.
        - If `stations.size() < 2`, skip rendering this line (it's not a valid segment).
            - `TDD_ANCHOR: Test_LineRender_Skip_LineTooShort`
        - Get the line color: `lineColor = lineComp.color`.
        - Create a `sf::VertexArray` for this line, typically `sf::LinesStrip` or `sf::Lines`. Using `sf::LinesStrip` is often simpler for continuous lines.
            - `vertexArray.setPrimitiveType(sf::LinesStrip)`
        - **Iterate Stations:** Loop through the `stations` vector from index 0 to `stations.size() - 1`.
            - Get the current station entity: `stationEntity = stations[i]`.
            - Try to get its `PositionComponent` (`stationPos`). If invalid, log a warning and potentially skip this segment or the entire line.
                - `TDD_ANCHOR: Test_LineRender_Handle_InvalidStationInLine`
            - Create a vertex: `sf::Vertex vertex(sf::Vector2f(stationPos.x, stationPos.y), lineColor)`.
            - Append the vertex to the `vertexArray`.
            - `TDD_ANCHOR: Test_LineRender_AppendVertex`
        - **Draw Line:** If `vertexArray` has at least 2 vertices, call `RenderingSystem.draw(vertexArray)`.
            - `TDD_ANCHOR: Test_LineRender_DrawLineCall`
        - **Line Thickness (Alternative):** If thickness > 1px is needed, `sf::LinesStrip` won't work directly. Instead, for each segment (between station `i` and `i+1`):
            - Calculate the perpendicular vector to the segment direction.
            - Create two triangles (a quad) forming a thick line segment using the station positions offset by the perpendicular vector scaled by half the thickness.
            - Add the 4 vertices of the quad (with `lineColor`) to a `sf::VertexArray(sf::Quads)` or `sf::VertexArray(sf::Triangles)`.
            - Draw this vertex array. This is more complex but allows thickness.
            - `TDD_ANCHOR: Test_LineRender_DrawThickLineSegment`

2.  **Draw Preview Line (If Active):**
    - Check the state of the `LineDrawingSystem` (e.g., access its `isDrawing` flag and `previewLinePoints`).
    - If `LineDrawingSystem.isDrawing == true` and `previewLinePoints` has at least two points:
        - Get the color of the line being drawn (`lineComp.color` from `LineDrawingSystem.currentLineEntity`, or a default preview color).
        - Create a `sf::VertexArray(sf::Lines)` containing two vertices: one for the start point and one for the end point from `previewLinePoints`. Set their color.
        - Call `RenderingSystem.draw(previewVertexArray)`.
        - `TDD_ANCHOR: Test_LineRender_DrawPreviewLine_Active`
    - Else:
        - Do not draw a preview line.
        - `TDD_ANCHOR: Test_LineRender_DrawPreviewLine_Inactive`

## 4. Edge Cases & Considerations

- **Performance:** Drawing many lines segment by segment using individual `draw` calls or complex thick line geometry can be slow. Using a single `sf::VertexArray` per line (with `sf::LinesStrip`) is generally efficient for thin lines. Batching vertices for multiple lines into fewer vertex arrays could further optimize if needed.
- **Line Thickness:** Implementing thick lines adds complexity. Consider if it's essential for MVP. SFML doesn't have built-in thick line support beyond 1px for `sf::Lines/sf::LinesStrip`.
- **Z-Ordering:** Ensure lines render underneath stations and vehicles, or according to desired layering.
- **Station Validity:** Gracefully handle cases where a station entity listed in a `LineComponent` has been deleted or lost its `PositionComponent`.
- **Anti-Aliasing:** SFML's default shapes/lines have limited anti-aliasing. For smoother lines, shaders or more advanced rendering techniques might be needed later.

## 5. TDD Anchors Summary

- `Test_LineRender_Skip_LineTooShort`
- `Test_LineRender_Handle_InvalidStationInLine`
- `Test_LineRender_AppendVertex`
- `Test_LineRender_DrawLineCall`
- `Test_LineRender_DrawThickLineSegment` (Optional/Alternative)
- `Test_LineRender_DrawPreviewLine_Active`
- `Test_LineRender_DrawPreviewLine_Inactive`