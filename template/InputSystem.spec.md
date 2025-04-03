// File: specs/InputSystem.spec.md
// Module: InputSystem
// Description: Processes raw input events, manages input state (keyboard, mouse), and potentially handles action mapping.

# Specification: Input System

## 1. Overview

Captures raw input events from the operating system (via SFML, forwarded by `RenderingSystem`) and/or ImGui. It maintains the current state of input devices (keyboard, mouse) and provides methods for other systems to query this state or check for specific input actions.

## 2. Dependencies

- SFML library (for event types and key codes)
- ImGui library (for UI input handling)
- `RenderingSystem`: To receive polled SFML events and potentially the window reference.
- (Potentially) `ConfigSystem`: To load input mappings (key bindings).
- (Potentially) `LoggingSystem`: For logging input-related information.

## 3. Data Structures

- `KeyState`: Enum (e.g., `UP`, `PRESSED`, `DOWN`, `RELEASED`) - Tracks state per key/button per frame.
- `KeyboardState`: Map or array storing the `KeyState` for each `sf::Keyboard::Key`.
- `MouseButtonState`: Map or array storing the `KeyState` for each `sf::Mouse::Button`.
- `MouseState`: Struct holding current position (screen and potentially world coordinates), wheel delta, and button states.
- `GameAction`: Enum defining abstract game actions (e.g., `MOVE_CAMERA_UP`, `SELECT`, `PAUSE_GAME`, `OPEN_MENU`).
- `InputMapping`: Map associating `GameAction` enums with specific key codes, mouse buttons, or combinations.
- `ActionState`: Map storing the `KeyState` for each `GameAction`.

## 4. Functions / Methods

### `initialize(configSystem, renderingSystem)`

1.  **Get Window Reference:** Obtain the `sf::RenderWindow` reference from `renderingSystem`.
2.  **Initialize State Maps:** Set all keys/buttons in `KeyboardState` and `MouseButtonState` to `UP`. Initialize `MouseState` (e.g., position to 0,0).
    - `TDD_ANCHOR: Test_Input_Initial_State`
3.  **Load Input Mappings (Optional):** Load key bindings from `ConfigSystem` into `InputMapping`. Define default mappings if loading fails or is not implemented.
    - `TDD_ANCHOR: Test_Input_Mapping_Load_Defaults`
    - `TDD_ANCHOR: Test_Input_Mapping_Load_FromFile`
4.  **Initialize ImGui IO:** Configure ImGui input handling flags if necessary (e.g., `io.WantCaptureKeyboard`, `io.WantCaptureMouse`).
5.  **Log Initialization:** Log successful initialization.

### `processEvents()` (Called once per frame, before updates)

1.  **Reset Per-Frame State:**
    - Update `KeyState`: Change `PRESSED` states to `DOWN`, `RELEASED` states to `UP`.
    - Reset mouse wheel delta.
    - `TDD_ANCHOR: Test_Input_State_Transition_FrameStart`
2.  **Poll Events (Alternative 1):** If `RenderingSystem` doesn't forward events, poll them here: `while (window.pollEvent(event)) { handleEvent(event); }`
3.  **Update Mouse Position:** Get current mouse position relative to the window (`sf::Mouse::getPosition(window)`). Store in `MouseState`.
    - Use `RenderingSystem.mapPixelToCoords` to calculate world position if needed.
    - `TDD_ANCHOR: Test_Input_Mouse_Position_Update`
    - `TDD_ANCHOR: Test_Input_Mouse_WorldPosition_Update`

### `handleEvent(event)` (Called by `processEvents` or `RenderingSystem`)

1.  **Forward to ImGui:** Pass the `event` to `ImGui::SFML::ProcessEvent(event)`.
2.  **Check ImGui Capture:** Check `ImGui::GetIO().WantCaptureKeyboard` and `ImGui::GetIO().WantCaptureMouse`. If ImGui wants the input, generally ignore it for game logic below (return early).
    - `TDD_ANCHOR: Test_Input_ImGui_Capture_Keyboard`
    - `TDD_ANCHOR: Test_Input_ImGui_Capture_Mouse`
3.  **Process SFML Event:** Based on `event.type`:
    - `KeyPressed`: If state was `UP` or `RELEASED`, set `KeyboardState[event.key.code]` to `PRESSED`.
        - `TDD_ANCHOR: Test_Input_Event_KeyPressed`
    - `KeyReleased`: Set `KeyboardState[event.key.code]` to `RELEASED`.
        - `TDD_ANCHOR: Test_Input_Event_KeyReleased`
    - `MouseButtonPressed`: If state was `UP` or `RELEASED`, set `MouseButtonState[event.mouseButton.button]` to `PRESSED`.
        - `TDD_ANCHOR: Test_Input_Event_MouseButtonPressed`
    - `MouseButtonReleased`: Set `MouseButtonState[event.mouseButton.button]` to `RELEASED`.
        - `TDD_ANCHOR: Test_Input_Event_MouseButtonReleased`
    - `MouseWheelScrolled`: Update `MouseState.wheelDelta`.
        - `TDD_ANCHOR: Test_Input_Event_MouseWheelScrolled`
    - `MouseMoved`: (Position already updated in `processEvents`).
    - Other events (TextEntered, Joystick, etc.) - Handle as needed.
        - `TDD_ANCHOR: Test_Input_Event_TextEntered`

### `updateActionStates()` (Called once per frame, after processing events)

1.  **Iterate Mappings:** Go through the `InputMapping`.
2.  **Check Input State:** For each `GameAction`, check the state of its mapped key(s)/button(s) in `KeyboardState` / `MouseButtonState`.
3.  **Update ActionState:** Update the `ActionState` map based on the input states (e.g., if mapped key is `PRESSED`, set action state to `PRESSED`). Handle complex mappings (e.g., Shift + Key) if implemented.
    - `TDD_ANCHOR: Test_Input_ActionState_Update`

### `isKeyDown(keyCode)` / `isKeyPressed(keyCode)` / `isKeyReleased(keyCode)`

1.  **Check KeyboardState:** Return `true` if `KeyboardState[keyCode]` matches `DOWN`/`PRESSED`/`RELEASED` respectively.
    - `TDD_ANCHOR: Test_Input_Query_KeyState`

### `isMouseButtonDown(button)` / `isMouseButtonPressed(button)` / `isMouseButtonReleased(button)`

1.  **Check MouseButtonState:** Return `true` if `MouseButtonState[button]` matches `DOWN`/`PRESSED`/`RELEASED` respectively.
    - `TDD_ANCHOR: Test_Input_Query_MouseButtonState`

### `getMousePositionScreen()` / `getMousePositionWorld()`

1.  **Return MouseState:** Return the stored screen or world coordinates from `MouseState`.
    - `TDD_ANCHOR: Test_Input_Query_MousePosition`

### `getMouseWheelDelta()`

1.  **Return MouseState:** Return the stored `wheelDelta` from `MouseState`.
    - `TDD_ANCHOR: Test_Input_Query_MouseWheelDelta`

### `isActionDown(action)` / `isActionPressed(action)` / `isActionReleased(action)`

1.  **Check ActionState:** Return `true` if `ActionState[action]` matches `DOWN`/`PRESSED`/`RELEASED` respectively. Requires `updateActionStates` to have been called.
    - `TDD_ANCHOR: Test_Input_Query_ActionState`

### `shutdown()`

1.  **Log Shutdown:** Log successful shutdown.
2.  **Cleanup:** Release any allocated resources.

## 5. Edge Cases & Considerations

- **ImGui Focus:** Correctly determining whether ImGui or the game world should handle input based on `WantCaptureMouse/Keyboard`.
- **Input Mapping Conflicts:** How to handle multiple actions mapped to the same key.
- **Device Hot-plugging:** Handling joysticks/controllers being connected/disconnected at runtime (likely out of scope initially).
- **Text Input:** Capturing text input for text fields requires handling `TextEntered` events carefully, especially regarding ImGui focus.
- **Multiple Windows:** If the application supports multiple windows, input needs to be associated with the correct window context.

## 6. TDD Anchors Summary

- `Test_Input_Initial_State`
- `Test_Input_Mapping_Load_Defaults`
- `Test_Input_Mapping_Load_FromFile`
- `Test_Input_State_Transition_FrameStart`
- `Test_Input_Mouse_Position_Update`
- `Test_Input_Mouse_WorldPosition_Update`
- `Test_Input_ImGui_Capture_Keyboard`
- `Test_Input_ImGui_Capture_Mouse`
- `Test_Input_Event_KeyPressed`
- `Test_Input_Event_KeyReleased`
- `Test_Input_Event_MouseButtonPressed`
- `Test_Input_Event_MouseButtonReleased`
- `Test_Input_Event_MouseWheelScrolled`
- `Test_Input_Event_TextEntered`
- `Test_Input_ActionState_Update`
- `Test_Input_Query_KeyState`
- `Test_Input_Query_MouseButtonState`
- `Test_Input_Query_MousePosition`
- `Test_Input_Query_MouseWheelDelta`
- `Test_Input_Query_ActionState`