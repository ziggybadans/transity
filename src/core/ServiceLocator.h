#pragma once

#include "../event/EventBus.h"
#include "Camera.h"
#include "EntityFactory.h"
#include "GameState.h"
#include "ThreadPool.h"
#include <entt/entt.hpp>

class Renderer;
class ColorManager;
class WorldGenerationSystem;

struct ServiceLocator {
    entt::registry *registry = nullptr;
    EventBus *eventBus = nullptr;
    GameState *gameState = nullptr;
    EntityFactory *entityFactory = nullptr;
    Camera *camera = nullptr;
    ColorManager *colorManager = nullptr;
    WorldGenerationSystem *worldGenerationSystem = nullptr;
    Renderer *renderer = nullptr;
    ThreadPool *threadPool = nullptr;
};
