# ECS Design Plan for Transity (using EnTT)

**I. Introduction & Goals**

*   **Primary Goal:** Define best practices and patterns for utilizing the EnTT library to implement a flexible, performant, and maintainable Entity-Component-System architecture for Transity.
*   **Initial Focus:** The "Mini Metro Clone" Minimum Viable Product (MVP), establishing a solid foundation.
*   **Balancing Act:** Strike a balance between modularity/flexibility for future expansion (as per `README.md` vision) and ease of use/simplicity for the initial development stages.
*   **Leverage EnTT:** Fully utilize EnTT's features for performance, type safety, and reducing boilerplate.

**II. Core EnTT Usage Principles**

1.  **Components as Plain Old Data (PODs):**
    *   Components will primarily be simple C++ structs or classes holding data, with minimal to no logic.
    *   Example: `struct Position { float x, y; };`
2.  **Systems for Behavior:**
    *   Logic and behavior will reside in Systems. Systems are functions or classes that operate on entities possessing specific sets of components.
    *   Systems will query the `entt::registry` for entities matching their required component signatures.
3.  **Entity Composition:**
    *   Entities are lightweight identifiers (`entt::entity`). Their "type" or "nature" is defined by the combination of components attached to them.
    *   This allows for flexible creation of diverse game objects.
4.  **`entt::registry` as Central Hub:**
    *   A single `entt::registry` instance will manage all entities, components, and provide views/groups for querying.
5.  **Leverage EnTT Features:**
    *   **Views:** Use `registry.view<ComponentA, ComponentB>()` for iterating over entities with specific components.
    *   **Groups:** For performance-critical iterations where component presence and order matter, consider `registry.group<ComponentA>(entt::get<ComponentB>)`.
    *   **Observers/Signals:** Utilize `registry.on_construct<T>()`, `on_update<T>()`, `on_destroy<T>()` for reactive logic when components are added, modified, or removed. This is key for decoupled inter-system communication.
    *   **Context Variables:** EnTT's context variables can be used to store global-like system data or references if needed, accessible by systems.

**III. MVP Component Design (Initial Set)**

*   **`IdentifierComponent`**: `std::string name; uint32_t id;` (for debugging, potentially UI)
*   **`PositionComponent`**: `float x, y;` (for stations, trains, potentially passengers if rendered individually)
*   **`RenderableComponent`**: `ShapeType shape; Color color; int z_order;` (for anything drawn)
*   **`ClickableComponent`**: `float bounding_radius;` (for stations, UI elements)

*   **`StationComponent`**:
    *   `StationType type;` (e.g., circle, square, triangle - dictates shape via `RenderableComponent`)
    *   `std::vector<entt::entity> waiting_passengers;`
    *   `std::vector<entt::entity> connected_lines;`
*   **`LineComponent`**:
    *   `Color color;` (can also be in `RenderableComponent`)
    *   `std::vector<entt::entity> stops;` (ordered list of Station entities)
    *   `std::vector<entt::entity> trains_on_line;`
*   **`PassengerComponent`**:
    *   `entt::entity destination_station;`
    *   `entt::entity current_station_or_train;` (entity they are currently at/on)
    *   `std::vector<entt::entity> path_segments;` (e.g., sequence of lines or stations to traverse)
    *   `PassengerState state;` (e.g., WAITING, ON_TRAIN, ARRIVED)
*   **`TrainComponent`**:
    *   `int capacity;`
    *   `int current_load;`
    *   `entt::entity assigned_line;`
    *   `float speed;`
    *   `int num_carriages;` (influences capacity and visual representation)
    *   `float progress_on_segment;` (0.0 to 1.0 between current and next stop)
    *   `int current_segment_index;` (index into `LineComponent::stops`)
    *   `TrainState state;` (e.g., MOVING, LOADING, AT_STATION)
*   **`PathComponent`**: (Added to passengers by pathfinding)
    *   `std::vector<entt::entity> nodes;` // Sequence of stations/lines
    *   `int current_node_index;`

**IV. MVP System Design (Initial Set & Order)**

Systems will be updated in a defined order each game loop tick.

1.  **`InputSystem`**:
    *   Responsibilities: Processes raw user input (mouse clicks, keyboard).
    *   Interactions: Creates "intent" components or directly triggers actions like placing a station or drawing a line segment. For MVP, might directly create station entities or add to a temporary "line being drawn" state.
2.  **`StationManagementSystem`**:
    *   Responsibilities: Handles logic related to stations, e.g., passenger spawning (could be a separate system), managing passenger queues at stations.
    *   Operates on: Entities with `StationComponent`.
3.  **`LineManagementSystem`**:
    *   Responsibilities: Finalizes line creation from input, updates line data (e.g., if a station is deleted).
    *   Operates on: Entities with `LineComponent`, `StationComponent`.
4.  **`PassengerSpawnSystem`**:
    *   Responsibilities: Periodically spawns new `Passenger` entities at `Station` entities, assigning them a `DestinationStation`.
    *   Operates on: `StationComponent` (to know where to spawn), creates `PassengerComponent`.
5.  **`PassengerPathfindingSystem`**:
    *   Responsibilities: For passengers needing a path (e.g., new passengers, or if their current path is invalidated), calculate a route to their `DestinationStation`. Adds/updates a `PathComponent` on the passenger entity.
    *   Operates on: Entities with `PassengerComponent` (and no/invalid `PathComponent`).
    *   *Note: For MVP, pathfinding can be simple (e.g., fewest transfers or shortest number of stops).*
6.  **`TrainMovementSystem`**:
    *   Responsibilities: Moves trains along their `AssignedLine` based on `Speed` and `progress_on_segment`. Updates `TrainComponent::current_segment_index` and `progress_on_segment`. Handles arrival at stations.
    *   Operates on: Entities with `TrainComponent`, `PositionComponent`, `LineComponent` (of assigned line).
7.  **`PassengerBoardingSystem`**: (Could be part of `TrainSystem` or `StationSystem` initially)
    *   Responsibilities: Manages passengers moving from a `StationComponent::waiting_passengers` list to a `TrainComponent::current_load` if the train is at the station, has capacity, and is on the passenger's path. Updates `PassengerComponent::current_station_or_train` and `PassengerComponent::state`.
    *   Operates on: `PassengerComponent`, `TrainComponent`, `StationComponent`.
8.  **`PassengerAlightingSystem`**: (Could be part of `TrainSystem` or `StationSystem` initially)
    *   Responsibilities: Manages passengers moving from a `TrainComponent` to a `StationComponent` if the train is at their `DestinationStation` or a transfer station on their `PathComponent`.
    *   Operates on: `PassengerComponent`, `TrainComponent`, `StationComponent`.
9.  **`RenderSystem`**:
    *   Responsibilities: Iterates over all entities with `PositionComponent` and `RenderableComponent` and draws them. Draws lines based on `LineComponent` and station positions.
    *   Operates on: Entities with `PositionComponent`, `RenderableComponent`, `LineComponent`, `TrainComponent` (for train visuals).

**V. Inter-System Communication Strategy**

*   **Primary Method: Ordered System Updates & Component Queries:**
    *   Systems run in a defined sequence. A system can rely on data prepared by previous systems in the same tick.
    *   Example: `PassengerPathfindingSystem` adds/updates a `PathComponent`. The `PassengerBoardingSystem` (later in the update order) queries for passengers with a valid `PathComponent` at a station where a train has arrived.
*   **Secondary Method (for more decoupled/reactive events): EnTT Observers/Signals:**
    *   Use `registry.on_construct<PathComponent>()` to trigger logic when a passenger gets a new path (e.g., update UI, play a sound).
    *   Use `registry.on_update<TrainComponent>()` if a specific change in train state (e.g., `current_load` changes) needs to trigger an immediate, decoupled reaction elsewhere.
    *   This approach keeps systems decoupled without the need for a complex manual event bus, fitting the "modularized and decoupled enough... but not enough to cause headaches" requirement.
*   **Avoid for MVP:** A heavy, custom event bus/messaging system. EnTT's built-in features should suffice and keep complexity lower.

**VI. Entity Lifecycle Management**

*   **Creation:**
    *   Use `entt::registry::create()` to get a new entity ID.
    *   Use `registry.emplace<ComponentType>(entity, constructor_args...)` to add components.
    *   The existing `EntityFactory.h`/`EntityFactory.cpp` (from your open tabs) should be adapted to use the EnTT registry for creating pre-configured entities (e.g., `createTrain(registry, line_entity, capacity)`).
*   **Updating:** Systems directly modify component data for entities they process.
*   **Destruction:**
    *   Use `entt::registry::destroy(entity)` to remove an entity and all its components.
    *   EnTT observers (`on_destroy`) can handle any custom cleanup logic if needed when specific components are removed as part of entity destruction.

**VII. Addressing Performance (Passenger Simulation)**

*   **Efficient Queries:** EnTT's views and groups are designed for fast iteration. The `PassengerPathfindingSystem` and `TrainMovementSystem` will benefit most.
*   **Data Locality:** EnTT stores components of the same type contiguously, leading to cache-friendly access, which is crucial for iterating over many passengers or trains.
*   **Pathfinding Algorithm:** The ECS provides the *structure* for pathfinding data. The efficiency of the pathfinding *algorithm itself* within `PassengerPathfindingSystem` will be critical and can be iterated upon (from simple to more complex A* as needed).
*   **Minimize Per-Passenger Work:** Design systems to do bulk updates where possible.

**VIII. Modularity and Future Growth**

*   **Adding Features:** New game mechanics (e.g., different vehicle types, cargo, economic factors) can be added by defining new components and new systems that operate on them. Existing systems remain largely untouched if the new features don't directly modify their core components.
*   **Modding Support (`README.md` line 24, 51):** A clean ECS is inherently mod-friendly. Mods can:
    *   Define new components.
    *   Define new systems.
    *   Register their systems with the game's main loop.
    *   Add new data to existing component types (if components are designed to be extensible, e.g., using `std::map` or `entt::any`).
*   **Refactoring:** Encapsulation of logic within systems and data within components makes it easier to refactor or replace parts of the game without widespread changes.

**IX. Mermaid Diagram (Conceptual MVP)**

```mermaid
graph TD
    subgraph Entities
        E_Station[Station Entity]
        E_Line[Line Entity]
        E_Passenger[Passenger Entity]
        E_Train[Train Entity]
    end

    subgraph Components
        C_Position[PositionComponent]
        C_Renderable[RenderableComponent]
        C_Clickable[ClickableComponent]
        C_Station[StationComponent]
        C_Line[LineComponent]
        C_Passenger[PassengerComponent]
        C_Train[TrainComponent]
        C_Path[PathComponent]
    end

    subgraph Systems
        S_Input[InputSystem]
        S_StationMgmt[StationManagementSystem]
        S_LineMgmt[LineManagementSystem]
        S_PassengerSpawn[PassengerSpawnSystem]
        S_Pathfinding[PassengerPathfindingSystem]
        S_TrainMove[TrainMovementSystem]
        S_Boarding[PassengerBoardingSystem]
        S_Alighting[PassengerAlightingSystem]
        S_Render[RenderSystem]
    end

    %% Entity to Component Mappings
    E_Station --- C_Position
    E_Station --- C_Renderable
    E_Station --- C_Clickable
    E_Station --- C_Station

    E_Line --- C_Line
    E_Line --- C_Renderable %% For drawing the line itself

    E_Passenger --- C_Passenger
    E_Passenger --- C_Position %% If rendered, or for distance checks
    E_Passenger --- C_Path

    E_Train --- C_Train
    E_Train --- C_Position
    E_Train --- C_Renderable

    %% System to Component Interactions (Simplified)
    S_Input --> C_Clickable %% Reads clicks
    S_Input --> E_Station %% Creates Stations (example)
    S_Input --> E_Line %% Creates Lines (example)

    S_StationMgmt --> C_Station
    S_StationMgmt --> C_Passenger %% Manages waiting passengers

    S_LineMgmt --> C_Line
    S_LineMgmt --> C_Station %% Uses station positions

    S_PassengerSpawn --> C_Station %% Spawns at stations
    S_PassengerSpawn --> E_Passenger %% Creates Passengers

    S_Pathfinding --> C_Passenger
    S_Pathfinding --> C_Station %% Needs station graph
    S_Pathfinding --> C_Line %% Needs line graph
    S_Pathfinding -.-> C_Path %% Writes to PathComponent

    S_TrainMove --> C_Train
    S_TrainMove --> C_Position %% Updates train position
    S_TrainMove --> C_Line %% Reads line stops

    S_Boarding --> C_Passenger
    S_Boarding --> C_Train
    S_Boarding --> C_Station

    S_Alighting --> C_Passenger
    S_Alighting --> C_Train
    S_Alighting --> C_Station

    S_Render --> C_Position
    S_Render --> C_Renderable
    S_Render --> C_Line
```

**Appendix A: On the Use of Tags (Empty Components)**

In EnTT (and many ECS architectures), "tags" are essentially empty components. You'd define an empty struct, say `struct IsSelectedTag {};`, and then you can add this tag to an entity like any other component: `registry.emplace<IsSelectedTag>(entity);`.

**How Tags Compare to Components with Data:**

*   **Tags (Empty Components):**
    *   **Purpose:** Primarily for marking or categorizing entities without associating any specific data with that mark. For example, `IsStaticTag`, `IsPlayerTag`, `IsVisibleTag`.
    *   **Pros:** Very lightweight (no data storage per entity beyond tracking its presence). Queries for entities *having* a tag can be very fast (`registry.view<IsSelectedTag>()`).
    *   **Cons:** If you later realize you need data associated with that "tag" (e.g., for `IsVisibleTag`, you might want a `transparency_level`), you'd need to refactor it into a component with data.

*   **Components with Data (like those in this document):**
    *   **Purpose:** To store data that defines an entity's attributes or state.
    *   **Pros:** Directly associate data with an aspect of an entity.
    *   **Cons:** Slightly more overhead than a tag if the component *only* serves as a marker and its data is rarely used or always default.

**Are Tags Useful for This Project, or Are Current Components Enough?**

For the "Mini Metro Clone" MVP and the components currently listed, the existing components largely serve the purpose of "tagging" an entity's type or core characteristics already:

1.  **Identifying Entity Types:**
    *   An entity having a `StationComponent` *is* a station. We don't necessarily need an additional `StationTag`. The presence of `StationComponent` (which holds vital station data) is the identifier.
    *   Similarly for `LineComponent`, `PassengerComponent`, `TrainComponent`.

2.  **Boolean States/Characteristics:**
    *   `ClickableComponent`: While it has a `bounding_radius`, its mere presence indicates the entity is clickable. If it had no data, it could be `ClickableTag`. Since it *does* have data relevant to its "clickability," it's better as a component.
    *   `RenderableComponent`: Its presence means "this should be rendered." It also holds the data *how* it should be rendered.

**Potential Uses for Tags (Now or Later):**

While the current list is quite robust for the MVP, tags could become useful for:

*   **Transient States:**
    *   `SelectedByPlayerTag`: To mark an entity (e.g., a station or train) that the player has currently selected in the UI. This is a common use case for a tag.
    *   `RequiresPathRecalculationTag`: If a change occurs that invalidates a passenger's path, this tag could mark them for the `PassengerPathfindingSystem`. (Though EnTT's `on_update` signals for relevant components might also handle this).
*   **System-Specific Markers:**
    *   `ProcessedBySystemXTag`: If a system should only process an entity once per complex update cycle.
*   **Filtering for Views:**
    *   If you have many types of, say, "physical objects" and want to quickly get all of them without listing every possible physical component type in a view. You could add a `PhysicalObjectTag` to all of them.

**Recommendation for MVP:**

For the initial "Mini Metro Clone" phase, the current component list is likely **sufficient and promotes clarity**. The components as defined already provide strong typing and data association. Introducing tags for things already covered by the presence of a data-holding component might add a layer of indirection without significant benefit *at this stage*.

The goal is to strike a balance between flexibility/modularity and ease of use. For a small game's MVP, leaning towards fewer, more data-rich components (where data is actually needed) is often easier to manage than a proliferation of many fine-grained tags.

We can always introduce tags later if specific needs arise where they offer a clear advantage (like the `SelectedByPlayerTag` example). EnTT makes it easy to add new component types (including tags) as the project evolves.