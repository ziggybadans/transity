// File: specs/ECSCore.spec.md
// Module: ECSCore
// Description: Manages the Entity-Component-System architecture using the EnTT library.

# Specification: ECS Core

## 1. Overview

This module provides the core functionalities of the Entity-Component-System (ECS) architecture, acting as a central registry and manager for entities, components, and systems. It leverages the EnTT library for efficient implementation.

## 2. Dependencies

- EnTT library (External C++ library)
- Component definitions (Defined elsewhere, e.g., `specs/components/*.spec.md`)
- System definitions (Defined elsewhere, e.g., `specs/systems/*.spec.md`)
- (Potentially) `LoggingSystem`: For logging ECS-related events or errors.

## 3. Core Concepts & Data Structures (Leveraging EnTT)

- **Registry (`entt::registry`):** The central EnTT object that owns and manages all entities and components. The `ECSCore` module will likely encapsulate or provide access to one or more registries.
- **Entity (`entt::entity`):** A simple identifier (integer). Represents an object in the game world (e.g., a vehicle, a station, a cim).
- **Component:** Plain data structures (structs/classes) holding data for a specific aspect of an entity (e.g., `PositionComponent`, `VelocityComponent`, `RenderableComponent`, `PassengerComponent`). Components are defined separately.
- **System:** Logic that operates on entities possessing specific sets of components (e.g., `MovementSystem` operates on entities with `PositionComponent` and `VelocityComponent`). Systems are defined separately. Systems can be categorized (e.g., Update Systems, Render Systems).

## 4. Functions / Methods (Wrapping or Utilizing EnTT Registry)

### `initialize()`

1.  **Create Registry:** Instantiate the main `entt::registry` object.
    - `TDD_ANCHOR: Test_ECSCore_Registry_Creation`
2.  **Register Systems (Optional):** Maintain internal lists or maps to store instances of different system types (e.g., update systems, render systems). Systems might be instantiated here or registered dynamically later.
    - `TDD_ANCHOR: Test_ECSCore_System_Registration`
3.  **Log Initialization:** Log successful initialization of the ECS core.

### `createEntity()`

1.  **Create via Registry:** Use `registry.create()` to get a new entity ID.
2.  **Return Entity:** Return the `entt::entity`.
    - `TDD_ANCHOR: Test_ECSCore_Entity_Creation`

### `destroyEntity(entity)`

1.  **Destroy via Registry:** Use `registry.destroy(entity)`. Ensure this handles component cleanup correctly (EnTT does this automatically).
    - `TDD_ANCHOR: Test_ECSCore_Entity_Destruction`
    - `TDD_ANCHOR: Test_ECSCore_Entity_Destruction_Removes_Components`

### `addComponent<T>(entity, ...args)`

1.  **Add via Registry:** Use `registry.emplace<T>(entity, ...args)` or `registry.assign<T>(entity, ...args)` to add a component of type `T` to the entity, constructing it with `args`.
    - Handle cases where the component already exists if necessary (e.g., `emplace_or_replace`).
    - `TDD_ANCHOR: Test_ECSCore_Component_Addition`
    - `TDD_ANCHOR: Test_ECSCore_Component_Addition_Replace`

### `removeComponent<T>(entity)`

1.  **Remove via Registry:** Use `registry.remove<T>(entity)`.
    - `TDD_ANCHOR: Test_ECSCore_Component_Removal`

### `getComponent<T>(entity)`

1.  **Get via Registry:** Use `registry.get<T>(entity)` to retrieve a reference to the component.
    - Handles cases where the component doesn't exist (EnTT throws, may need try/catch or `try_get`).
    - `TDD_ANCHOR: Test_ECSCore_Component_Retrieval_Exists`
    - `TDD_ANCHOR: Test_ECSCore_Component_Retrieval_NotExists` (Test `try_get` or exception handling)

### `hasComponent<T>(entity)`

1.  **Check via Registry:** Use `registry.has<T>(entity)` or `registry.all_of<T>(entity)`.
    - `TDD_ANCHOR: Test_ECSCore_Component_Check_Has`
    - `TDD_ANCHOR: Test_ECSCore_Component_Check_NotHas`

### `getView<T...>()`

1.  **View via Registry:** Use `registry.view<T...>()` to get an iterable view of all entities possessing the specified component types `T...`. This is the primary way systems query entities.
    - `TDD_ANCHOR: Test_ECSCore_View_SingleComponent`
    - `TDD_ANCHOR: Test_ECSCore_View_MultiComponent`
    - `TDD_ANCHOR: Test_ECSCore_View_Empty`

### `updateSystems(deltaTime)`

1.  **Iterate Update Systems:** Loop through all registered "update" systems.
2.  **Execute System:** Call an `update(registry, deltaTime)` method on each system instance. The system itself will use `registry.view()` to find relevant entities.
    - The order of system execution might be important and needs to be defined/managed.
    - `TDD_ANCHOR: Test_ECSCore_UpdateSystems_Execution`
    - `TDD_ANCHOR: Test_ECSCore_UpdateSystems_Order`

### `renderSystems(renderer)`

1.  **Iterate Render Systems:** Loop through all registered "render" systems.
2.  **Execute System:** Call a `render(registry, renderer)` method on each system instance. The system uses views to get entities and their components (e.g., `PositionComponent`, `RenderableComponent`) to draw them using the `renderer` object.
    - `TDD_ANCHOR: Test_ECSCore_RenderSystems_Execution`

### `shutdown()`

1.  **Clear Registry:** Call `registry.clear()` to destroy all entities and components.
    - `TDD_ANCHOR: Test_ECSCore_Shutdown_RegistryClear`
2.  **Cleanup Systems:** Delete or clean up any stored system instances if dynamically allocated.
3.  **Log Shutdown:** Log successful shutdown.

## 5. Edge Cases & Considerations

- **System Execution Order:** Define how the order of system updates is determined (e.g., registration order, explicit dependency graph). Incorrect order can lead to bugs.
- **Component/Entity Lifecycles:** Ensure components are correctly added/removed when entities are created/destroyed. EnTT handles much of this, but complex dependencies might need care.
- **Registry Scope:** Decide if a single global registry is sufficient or if multiple registries are needed (e.g., one for UI, one for the game world). Start with one.
- **Performance:** Views are efficient, but complex systems or iterating over huge numbers of entities still require optimization considerations (e.g., spatial partitioning managed by a dedicated system).
- **Data Locality:** ECS promotes data locality, but component layout and access patterns within systems still matter for cache performance.

## 6. TDD Anchors Summary

- `Test_ECSCore_Registry_Creation`
- `Test_ECSCore_System_Registration`
- `Test_ECSCore_Entity_Creation`
- `Test_ECSCore_Entity_Destruction`
- `Test_ECSCore_Entity_Destruction_Removes_Components`
- `Test_ECSCore_Component_Addition`
- `Test_ECSCore_Component_Addition_Replace`
- `Test_ECSCore_Component_Removal`
- `Test_ECSCore_Component_Retrieval_Exists`
- `Test_ECSCore_Component_Retrieval_NotExists`
- `Test_ECSCore_Component_Check_Has`
- `Test_ECSCore_Component_Check_NotHas`
- `Test_ECSCore_View_SingleComponent`
- `Test_ECSCore_View_MultiComponent`
- `Test_ECSCore_View_Empty`
- `Test_ECSCore_UpdateSystems_Execution`
- `Test_ECSCore_UpdateSystems_Order`
- `Test_ECSCore_RenderSystems_Execution`
- `Test_ECSCore_Shutdown_RegistryClear`