#pragma once

#include "entt/entt.hpp"

class InputHandler;
class UI;
class EntityFactory;

class StationPlacementSystem {
public:
    void update(InputHandler& inputHandler, UI& ui, entt::registry& registry, EntityFactory& entityFactory);
};