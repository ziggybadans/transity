#pragma once

#include "ecs/ISystem.h"
#include "app/GameState.h"
#include <entt/entt.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class LineEditingRenderSystem : public ISystem {
public:
    LineEditingRenderSystem(sf::RenderWindow& window);
    ~LineEditingRenderSystem();

    void draw(entt::registry& registry, GameState& gameState);

private:
    sf::RenderWindow& _window;
};