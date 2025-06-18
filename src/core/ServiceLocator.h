#pragma once

#include <entt/entt.hpp>
#include "../event/EventBus.h"
#include "GameState.h"
#include "EntityFactory.h"
#include "Camera.h"
#include "../graphics/ColorManager.h"
#include "../graphics/Renderer.h"

// Forward declarations to avoid circular dependencies if needed
class Renderer;

struct ServiceLocator {
    entt::registry* registry = nullptr;
    EventBus* eventBus = nullptr;
    GameState* gameState = nullptr;
    EntityFactory* entityFactory = nullptr;
    Camera* camera = nullptr;
    ColorManager* colorManager = nullptr;
    Renderer* renderer = nullptr;
};
