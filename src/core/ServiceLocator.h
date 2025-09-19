#pragma once

#include "ThreadPool.h"
#include "app/GameState.h"
#include "app/LoadingState.h"
#include "core/PerformanceMonitor.h"
#include "ecs/EntityFactory.h"
#include "event/EventBus.h"
#include "render/Camera.h"
#include <entt/entt.hpp>

class Renderer;
class ColorManager;
class WorldGenerationSystem;
class Pathfinder;

struct ServiceLocator {
    entt::registry &registry;
    EventBus &eventBus;
    LoadingState &loadingState;
    GameState &gameState;
    EntityFactory &entityFactory;
    Camera &camera;
    ColorManager &colorManager;
    WorldGenerationSystem &worldGenerationSystem;
    Renderer &renderer;
    ThreadPool &threadPool;
    Pathfinder &pathfinder;
    PerformanceMonitor &performanceMonitor;
};