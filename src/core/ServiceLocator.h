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
    entt::registry &registry;
    EventBus &eventBus;
    GameState &gameState;
    EntityFactory &entityFactory;
    Camera &camera;
    ColorManager &colorManager;
    WorldGenerationSystem &worldGenerationSystem;
    Renderer &renderer;
    ThreadPool &threadPool;
};