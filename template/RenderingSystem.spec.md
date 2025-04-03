// File: specs/RenderingSystem.spec.md
// Module: RenderingSystem
// Description: Handles window creation, rendering using SFML, view management, and basic drawing.

# Specification: Rendering System

## 1. Overview

Manages the application window, viewport, camera/view, and all drawing operations using the SFML graphics library. It provides an interface for other systems (primarily ECS render systems) to draw objects onto the screen.

## 2. Dependencies

- SFML library (External C++ library)
- `ConfigSystem`: To get window settings (size, title, vsync, etc.).
- `InputSystem`: Potentially needed to pass window events (like resize) to the input system, or vice-versa.
- (Potentially) `LoggingSystem`: For logging rendering-related info or errors.
- (Potentially) `ResourceManager`: For loading and managing textures, fonts, shaders (May be a separate spec).

## 3. Data Structures

- `sf::RenderWindow`: The main SFML window object.
- `sf::View`: The SFML view object representing the camera, controlling zoom and position.
- `WindowConfig`: Struct holding window parameters (width, height, title, style flags, vsync).
- `Color`: Wrapper or alias for `sf::Color`.

## 4. Functions / Methods

### `initialize(configSystem)`

1.  **Load Configuration:** Get window settings (width, height, title, fullscreen, vsync, framerate limit) from `ConfigSystem`.
    - `TDD_ANCHOR: Test_Rendering_Config_Load`
2.  **Create Window:** Instantiate `sf::RenderWindow` using the loaded configuration.
    - Handle potential creation errors (e.g., unsupported video mode).
    - `TDD_ANCHOR: Test_Rendering_Window_Creation_Success`
    - `TDD_ANCHOR: Test_Rendering_Window_Creation_Error`
3.  **Apply Settings:** Set vsync (`window.setVerticalSyncEnabled`) and framerate limit (`window.setFramerateLimit`) based on config.
    - `TDD_ANCHOR: Test_Rendering_VSync_Setting`
    - `TDD_ANCHOR: Test_Rendering_FramerateLimit_Setting`
4.  **Initialize View:** Create the main `sf::View`. Set its initial size (matching window dimensions) and center (e.g., 0,0 or center of initial world).
    - `TDD_ANCHOR: Test_Rendering_View_Initialization`
5.  **Initialize ImGui (If used directly here):** Set up ImGui-SFML bindings. (Alternatively, this could be in a dedicated `UISystem`).
    - `TDD_ANCHOR: Test_Rendering_ImGui_Initialization` (If applicable)
6.  **Log Initialization:** Log successful renderer and window initialization.

### `isWindowOpen()`

1.  **Check Window State:** Return `window.isOpen()`.
    - `TDD_ANCHOR: Test_Rendering_Window_IsOpen`

### `getWindowReference()`

1.  **Return Window:** Return a reference or pointer to the `sf::RenderWindow` object (needed by InputSystem, ImGui).
    - `TDD_ANCHOR: Test_Rendering_GetWindowReference`

### `processWindowEvents(inputSystem)`

1.  **Poll Events:** Poll SFML events from the window (`window.pollEvent(event)`).
2.  **Handle Resize:** If a `sf::Event::Resized` event occurs:
    - Update the main `sf::View` size (`view.setSize(event.size.width, event.size.height)`).
    - Apply the updated view (`window.setView(view)`).
    - `TDD_ANCHOR: Test_Rendering_Event_Resize`
3.  **Handle Close:** If a `sf::Event::Closed` event occurs, signal application exit (e.g., return a flag, call a callback). This might be handled directly in `CoreApplication` loop by checking `InputSystem` state instead.
4.  **Forward to Input/UI:** Pass other relevant events (keyboard, mouse, joystick, ImGui events) to the `InputSystem` or `UISystem` for processing.
    - `TDD_ANCHOR: Test_Rendering_Event_Forwarding`

### `clear(backgroundColor)`

1.  **Clear Window:** Call `window.clear(backgroundColor)`.
    - `TDD_ANCHOR: Test_Rendering_Clear_Screen`

### `draw(drawable, renderStates)`

1.  **Apply View:** Ensure the correct view is set (`window.setView(view)`). This might only be needed once per frame if the view doesn't change during drawing.
2.  **Draw Object:** Call `window.draw(drawable, renderStates)` where `drawable` is an `sf::Drawable` object (e.g., `sf::Sprite`, `sf::Text`, `sf::Shape`) and `renderStates` includes transforms, shaders, blend mode.
    - `TDD_ANCHOR: Test_Rendering_Draw_Sprite`
    - `TDD_ANCHOR: Test_Rendering_Draw_Shape`
    - `TDD_ANCHOR: Test_Rendering_Draw_Text`

### `display()`

1.  **Swap Buffers:** Call `window.display()`.
    - `TDD_ANCHOR: Test_Rendering_Display_Frame`

### `setViewCenter(x, y)`

1.  **Update View:** Set the center of the main `sf::View`.
    - `TDD_ANCHOR: Test_Rendering_View_SetCenter`

### `zoomView(factor)`

1.  **Update View:** Apply zoom factor to the main `sf::View` (`view.zoom(factor)`).
    - `TDD_ANCHOR: Test_Rendering_View_Zoom`

### `getView()`

1.  **Return View:** Return a copy or const reference to the current main `sf::View`.
    - `TDD_ANCHOR: Test_Rendering_View_Get`

### `mapPixelToCoords(pixelCoords)` / `mapCoordsToPixel(worldCoords)`

1.  **Coordinate Mapping:** Use `window.mapPixelToCoords(pixelCoords, view)` and `window.mapCoordsToPixel(worldCoords, view)` for converting between screen pixels and world coordinates based on the current view. Essential for UI interaction with the game world.
    - `TDD_ANCHOR: Test_Rendering_Coordinate_Mapping`

### `shutdown()`

1.  **Close Window:** Call `window.close()` if not already closed.
2.  **Cleanup Resources:** Release any resources managed directly by this system (if not using a separate `ResourceManager`).
3.  **Shutdown ImGui:** Clean up ImGui-SFML bindings.
    - `TDD_ANCHOR: Test_Rendering_ImGui_Shutdown` (If applicable)
4.  **Log Shutdown:** Log successful shutdown.

## 5. Resource Management (Textures, Fonts, Shaders)

- **Decision:** Will `RenderingSystem` manage loading/unloading directly, or will a dedicated `ResourceManager` handle this?
- **Recommendation:** A separate `ResourceManager` is generally better for preventing redundant loads and managing resource lifetimes. `RenderingSystem` would then request resources from the manager. For the initial spec, we can assume resources are loaded elsewhere or ad-hoc.

## 6. Edge Cases & Considerations

- **Window Resizing:** Ensure viewport and view are updated correctly on resize.
- **Fullscreen/Windowed Toggle:** Handle transitions between fullscreen and windowed mode (may require recreating the window or changing style flags).
- **Multiple Views:** Support for multiple views if needed (e.g., main world view + minimap view + UI view).
- **Render States:** Proper use of `sf::RenderStates` for transformations, shaders, and blending.
- **Performance:** Batching draw calls (less relevant with SFML compared to lower-level APIs, but still good practice), minimizing state changes. Culling off-screen objects.

## 7. TDD Anchors Summary

- `Test_Rendering_Config_Load`
- `Test_Rendering_Window_Creation_Success`
- `Test_Rendering_Window_Creation_Error`
- `Test_Rendering_VSync_Setting`
- `Test_Rendering_FramerateLimit_Setting`
- `Test_Rendering_View_Initialization`
- `Test_Rendering_ImGui_Initialization` (If applicable)
- `Test_Rendering_Window_IsOpen`
- `Test_Rendering_GetWindowReference`
- `Test_Rendering_Event_Resize`
- `Test_Rendering_Event_Forwarding`
- `Test_Rendering_Clear_Screen`
- `Test_Rendering_Draw_Sprite`
- `Test_Rendering_Draw_Shape`
- `Test_Rendering_Draw_Text`
- `Test_Rendering_Display_Frame`
- `Test_Rendering_View_SetCenter`
- `Test_Rendering_View_Zoom`
- `Test_Rendering_View_Get`
- `Test_Rendering_Coordinate_Mapping`
- `Test_Rendering_ImGui_Shutdown` (If applicable)