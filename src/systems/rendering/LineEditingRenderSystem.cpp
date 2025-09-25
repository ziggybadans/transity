#include "LineEditingRenderSystem.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
#include <SFML/Graphics/CircleShape.hpp>

LineEditingRenderSystem::LineEditingRenderSystem(sf::RenderWindow& window)
    : _window(window) {}

LineEditingRenderSystem::~LineEditingRenderSystem() = default;

void LineEditingRenderSystem::draw(entt::registry& registry, GameState& gameState) {
    if (gameState.currentInteractionMode != InteractionMode::EDIT_LINE) {
        return;
    }

    if (!gameState.selectedEntity.has_value()) {
        return;
    }

    entt::entity selectedLine = gameState.selectedEntity.value();
    if (!registry.all_of<LineComponent>(selectedLine)) {
        return;
    }

    const auto& line = registry.get<LineComponent>(selectedLine);

    for (const auto& point : line.points) {
        sf::CircleShape circle(5.f);
        circle.setOrigin({5.f, 5.f});
        circle.setPosition(point.position);
        circle.setFillColor(sf::Color::White);
        _window.draw(circle);
    }
}