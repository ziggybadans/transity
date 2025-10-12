#pragma once

#include "ecs/ISystem.h"
#include "app/GameState.h"
#include <entt/entt.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class LineEditingRenderSystem : public ISystem {
public:
    LineEditingRenderSystem();
    ~LineEditingRenderSystem();

    void draw(sf::RenderTarget& target, entt::registry& registry, GameState& gameState);

private:
};