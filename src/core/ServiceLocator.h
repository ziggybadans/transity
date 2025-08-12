#pragma once

#include "../event/EventBus.h"
#include "../graphics/ColorManager.h"
#include "../graphics/Renderer.h"
#include "Camera.h"
#include "EntityFactory.h"
#include "GameState.h"
#include <entt/entt.hpp>

// Forward declarations to avoid circular dependencies if needed
class Renderer;

struct ServiceLocator {
    entt::registry *registry = nullptr;
    EventBus *eventBus = nullptr;
    GameState *gameState = nullptr;
    EntityFactory *entityFactory = nullptr;
    Camera *camera = nullptr;
    ColorManager *colorManager = nullptr;
    Renderer *renderer = nullptr;
};
