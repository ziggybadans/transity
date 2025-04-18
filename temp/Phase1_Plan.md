# Development Plan: Phase 1 - Foundation

*Refer to the specification documents in `template/` for detailed requirements.*

**1. Logging System (`LoggingSystem.spec.md`)**
- [x] Define `LogLevel` enum.
- [x] Define `LogSink` interface (`ConsoleSink`, `FileSink`).
- [ ] Define `LogConfig` structure.
- [x] Implement `initialize(config)` function (load config, setup sinks, handle errors, log init).
- [x] Implement `log(level, message, ...args)` function (level filtering, formatting, dispatch).
- [x] Implement `shutdown()` function (flush/close sinks).
- [x] Implement `LogSink::write()` for `ConsoleSink`.
- [x] Implement `LogSink::write()` for `FileSink`.
- [x] (Optional) Implement helper macros (e.g., `LOG_INFO`).
- [x] Ensure basic thread safety.

**2. Configuration System (`ConfigSystem.spec.md`)**
- [ ] Decide on configuration file format (e.g., INI, JSON).
- [ ] Define `ConfigSource` enum, `ConfigValue` variant, `ConfigStore` map.
- [ ] Implement `initialize(...)` function (load defaults, parse chosen format, load primary/user files, handle errors).
- [ ] Implement `getValue<T>(key, defaultValue)` template function (lookup, type conversion, defaults).
- [ ] Implement convenience wrappers (`getString`, `getInt`, etc.).
- [ ] Implement `shutdown()` function (placeholder).

**3. ECS Core (`ECSCore.spec.md`)**
- [ ] Integrate EnTT library via build system (CMake/vcpkg).
- [ ] Create `ECSCore` module encapsulating `entt::registry`.
- [ ] Implement `initialize()` (create registry).
- [ ] Implement entity management wrappers (`createEntity`, `destroyEntity`).
- [ ] Implement component management wrappers (`addComponent`, `removeComponent`, `getComponent`, `hasComponent`).
- [ ] Implement view wrapper (`getView`).
- [ ] Implement system update/render loop placeholders (`updateSystems`, `renderSystems`).
- [ ] Implement `shutdown()` (clear registry).

**4. Rendering System (`RenderingSystem.spec.md`)**
- [ ] Integrate SFML library via build system.
- [ ] Integrate ImGui/ImGui-SFML libraries via build system.
- [ ] Create `RenderingSystem` module.
- [ ] Implement `initialize(configSystem)` (load config, create `sf::RenderWindow`, apply settings, create `sf::View`, init ImGui).
- [ ] Implement `isWindowOpen()`, `getWindowReference()`.
- [ ] Implement `processWindowEvents(inputSystem)` (poll events, handle resize/close, forward to ImGui/Input).
- [ ] Implement render loop methods (`clear`, `draw`, `display`).
- [ ] Implement view controls (`setViewCenter`, `zoomView`, `getView`).
- [ ] Implement coordinate mapping (`mapPixelToCoords`, `mapCoordsToPixel`).
- [ ] Implement `shutdown()` (close window, shutdown ImGui).

**5. Input System (`InputSystem.spec.md`)**
- [ ] Create `InputSystem` module.
- [ ] Define `KeyState` enum and state storage maps.
- [ ] Implement `initialize(configSystem, renderingSystem)` (get window ref, init state, load defaults).
- [ ] Implement `processEvents()` / `handleEvent(event)` (update internal state, process SFML events, check ImGui capture).
- [ ] Implement state query methods (`isKeyDown`, `isMouseButtonDown`, `getMousePositionScreen`, `getMouseWheelDelta`).
- [ ] (Optional) Implement Action Mapping (`GameAction`, `InputMapping`, `updateActionStates`, `isAction...`).
- [ ] Implement `shutdown()`.

**6. Core Application (`CoreApplication.spec.md`)**
- [ ] Create main entry point (`main()`).
- [ ] Implement initialization sequence in `main()` (Logging, Config, ECS, Rendering, Input).
- [ ] Implement `runMainLoop()` (window loop, process input, update systems (placeholder), render frame).
- [ ] Implement `shutdown()` sequence (call system shutdowns in reverse order).