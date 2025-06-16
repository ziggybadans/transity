#pragma once

#include "entt/entt.hpp"
#include "../input/InteractionMode.h" // Include the enum definition

class InputHandler;
class EntityFactory;

class StationPlacementSystem {
public:
    StationPlacementSystem(InputHandler& inputHandler, entt::registry& registry, EntityFactory& entityFactory);
    void update(InteractionMode mode);

private:
    InputHandler& _inputHandler;
    entt::registry& _registry;
    EntityFactory& _entityFactory;
};
