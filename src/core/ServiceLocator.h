#pragma once

#include "event/EventBus.h"
#include "render/Camera.h"
#include "ecs/EntityFactory.h"
#include "app/GameState.h"
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