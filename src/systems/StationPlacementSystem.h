#pragma once

#include "entt/entt.hpp"
#include "../input/InteractionMode.h" // Include the enum definition

class InputHandler;
class EntityFactory;

class StationPlacementSystem {
public:
    // The signature now takes the enum directly
    void update(InputHandler& inputHandler, InteractionMode mode, entt::registry& registry, EntityFactory& entityFactory);
};
