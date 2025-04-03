// File: specs/systems/LineDrawingSystem.spec.md
// Module: Systems
// System: LineDrawingSystem

# Specification: Line Drawing System

## 1. Overview

Handles player input (primarily mouse clicks) to create new transport lines or modify existing ones by connecting station entities.

## 2. Dependencies

- `ECSCore`: To create line entities, query stations, and modify `LineComponent`.
- `InputSystem`: To get mouse button states (`isMouseButtonPressed`, `isMouseButtonReleased`) and mouse position (`getMousePositionWorld`).
- `RenderingSystem`: To map screen coordinates to world coordinates (via `InputSystem`).
- `StationComponent`: To identify entities as stations that can be connected.
- `PositionComponent`: To get station positions for proximity checks and rendering previews.
- `LineComponent`: To add to new line entities and modify existing ones.
- (Potentially) `ResourceManager` / `ColorPalette`: To assign unique colors to new lines.
- (Potentially) `GameRuleSystem`: To check if the player is allowed to build (e.g., cost, limits - maybe later phase).
- (Potentially) `UISystem`: To avoid interaction when UI elements are clicked.

## 3. Data Structures (Internal State)

- `isDrawing`: Boolean flag, true if the player is currently dragging a line from a station.
- `startStationEntity`: `entt::entity` ID of the station where the current drawing action started.
- `currentLineEntity`: `entt::entity` ID of the line being actively modified or created. `entt::null` if not drawing.
- `previewLinePoints`: List of world coordinates for rendering a temporary line preview from `startStationEntity` to the current mouse position.

## 4. Logic (Update Cycle - `update(deltaTime)`)

1.  **Check UI Focus:** If `InputSystem` indicates UI has focus (e.g., `ImGui::GetIO().WantCaptureMouse`), return early to avoid interacting with the game world.
    - `TDD_ANCHOR: Test_LineDraw_Ignore_UIFocus`
2.  **Get Mouse State:** Get current world mouse position (`mouseWorldPos`) and mouse button states (e.g., left button pressed/released) from `InputSystem`.
3.  **Handle Mouse Press:** If `InputSystem.isMouseButtonPressed(LeftButton)`:
    - **Find Clicked Station:** Iterate through entities with `StationComponent` and `PositionComponent`. Check if `mouseWorldPos` is close enough to any station's position (using a click radius).
        - `TDD_ANCHOR: Test_LineDraw_FindStation_OnClick_Success`
        - `TDD_ANCHOR: Test_LineDraw_FindStation_OnClick_Miss`
    - **If Station Clicked (`clickedStationEntity`):**
        - **Check Existing Lines:** Does `clickedStationEntity` already belong to an existing line?
            - Iterate through entities with `LineComponent`. Check if `clickedStationEntity` is in their `stations` list.
        - **If Station is on a Line (`existingLineEntity`):**
            - Start modifying the existing line.
            - Set `isDrawing = true`.
            - Set `startStationEntity = clickedStationEntity`.
            - Set `currentLineEntity = existingLineEntity`.
            - Clear `previewLinePoints`.
            - `TDD_ANCHOR: Test_LineDraw_StartModify_ExistingLine`
        - **Else (Station is not on a Line):**
            - Start creating a new line.
            - Set `isDrawing = true`.
            - Set `startStationEntity = clickedStationEntity`.
            - Create a new entity for the line: `newLineEntity = ECSCore.createEntity()`.
            - Add `LineComponent` to `newLineEntity`. Assign a unique color (e.g., from a palette). Add `startStationEntity` to its `stations` list.
            - Set `currentLineEntity = newLineEntity`.
            - Clear `previewLinePoints`.
            - `TDD_ANCHOR: Test_LineDraw_StartCreate_NewLine`
            - `TDD_ANCHOR: Test_LineDraw_NewLine_AssignsColor`
            - `TDD_ANCHOR: Test_LineDraw_NewLine_AddsStartStation`
    - **Else (Clicked Empty Space):** Do nothing or handle deselection if applicable.
4.  **Handle Mouse Drag (While Drawing):** If `isDrawing == true`:
    - Update `previewLinePoints`: Start point is `startStationEntity`'s position, end point is `mouseWorldPos`.
    - (Render system will draw this preview line).
    - `TDD_ANCHOR: Test_LineDraw_UpdatePreviewLine`
5.  **Handle Mouse Release:** If `InputSystem.isMouseButtonReleased(LeftButton)` AND `isDrawing == true`:
    - **Find Target Station:** Check if `mouseWorldPos` is close enough to *another* station (`targetStationEntity`).
        - `TDD_ANCHOR: Test_LineDraw_FindTargetStation_OnRelease_Success`
    - **If Target Station Found AND `targetStationEntity != startStationEntity`:**
        - Get `LineComponent` (`lineComp`) for `currentLineEntity`.
        - **Check Connection Validity:** Can `startStationEntity` connect to `targetStationEntity` on this line? (e.g., prevent adding duplicate consecutive stations).
            - `TDD_ANCHOR: Test_LineDraw_CheckConnectionValidity`
        - **If Valid Connection:**
            - Add `targetStationEntity` to `lineComp.stations`. (Logic might be more complex: insert, append, handle merging if target is already on the *same* line, handle connecting to a *different* line - MVP likely just appends).
            - Log line modification/creation (e.g., "Connected Station A to Station B on Line Z").
            - `TDD_ANCHOR: Test_LineDraw_ConnectStations_Append`
            - // `TDD_ANCHOR: Test_LineDraw_ConnectStations_MergeSameLine` (Future)
            - // `TDD_ANCHOR: Test_LineDraw_ConnectStations_JoinDifferentLine` (Future)
    - **Else (Released on empty space or same station):**
        - If the line was newly created (`lineComp.stations.size() <= 1`), potentially destroy `currentLineEntity` as it's an invalid line.
        - `TDD_ANCHOR: Test_LineDraw_Cancel_InvalidNewLineDeletion`
    - **Reset State:**
        - Set `isDrawing = false`.
        - Set `startStationEntity = entt::null`.
        - Set `currentLineEntity = entt::null`.
        - Clear `previewLinePoints`.
        - `TDD_ANCHOR: Test_LineDraw_ResetState_OnRelease`

## 6. Rendering Preview (Handled by LineRenderSystem)

- The `LineRenderSystem` should check if `LineDrawingSystem.isDrawing == true`.
- If yes, it should draw a temporary line using the points in `LineDrawingSystem.previewLinePoints` and potentially the color of the `currentLineEntity`.

## 7. Edge Cases & Considerations

- **Click Radius:** Define a reasonable click tolerance around stations.
- **Line Modification Complexity:** Modifying existing lines (inserting stations, removing segments, merging lines) requires careful logic beyond simple appending. MVP might only support extending from line ends.
- **Crossing Lines:** Does the game allow lines to cross? MVP likely does.
- **Maximum Line Length/Stations:** Impose limits? (Maybe later).
- **Cost/Resources:** Add checks for construction costs? (Later phase).
- **Undo/Redo:** Support for undoing line modifications? (Later phase).
- **Visual Feedback:** Clear visual cues for start station, target station highlight, preview line.

## 8. TDD Anchors Summary

- `Test_LineDraw_Ignore_UIFocus`
- `Test_LineDraw_FindStation_OnClick_Success`
- `Test_LineDraw_FindStation_OnClick_Miss`
- `Test_LineDraw_StartModify_ExistingLine`
- `Test_LineDraw_StartCreate_NewLine`
- `Test_LineDraw_NewLine_AssignsColor`
- `Test_LineDraw_NewLine_AddsStartStation`
- `Test_LineDraw_UpdatePreviewLine`
- `Test_LineDraw_FindTargetStation_OnRelease_Success`
- `Test_LineDraw_CheckConnectionValidity`
- `Test_LineDraw_ConnectStations_Append`
- `Test_LineDraw_Cancel_InvalidNewLineDeletion`
- `Test_LineDraw_ResetState_OnRelease`
// Future/Complex Modification Anchors (Examples)
// - `Test_LineDraw_ConnectStations_MergeSameLine`
// - `Test_LineDraw_ConnectStations_JoinDifferentLine`
// - `Test_LineDraw_InsertStation_MidLine`
// - `Test_LineDraw_RemoveSegment`