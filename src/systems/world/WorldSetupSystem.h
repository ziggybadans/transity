#pragma once

#include "ecs/ISystem.h"
#include <entt/entt.hpp>

struct LoadingState;
class WorldGenerationSystem;
class Renderer;
class Camera;

class WorldSetupSystem : public ISystem, public IUpdatable {
public:
    explicit WorldSetupSystem(entt::registry& registry, LoadingState& loadingState, WorldGenerationSystem& worldGenerationSystem, Renderer& renderer, Camera& camera);
    ~WorldSetupSystem() override = default;

    void init();
    void update(sf::Time dt) override;

private:
    entt::registry& _registry;
    LoadingState& _loadingState;
    WorldGenerationSystem& _worldGenerationSystem;
    Renderer& _renderer;
    Camera& _camera;
};