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
    if (!registry.all_of<LineComponent, LineEditingComponent>(selectedLine)) {
        return;
    }

    const auto& line = registry.get<LineComponent>(selectedLine);
    const auto& editingState = registry.get<LineEditingComponent>(selectedLine);

    for (size_t i = 0; i < line.points.size(); ++i) {
        const auto& point = line.points[i];
        sf::CircleShape circle;
        circle.setPosition(point.position);

        if (editingState.selectedPointIndex.has_value() && editingState.selectedPointIndex.value() == i) {
            circle.setRadius(10.f);
            circle.setOrigin({10.f, 10.f});
            circle.setFillColor(sf::Color::Red);
        } else {
            circle.setRadius(8.f);
            circle.setOrigin({8.f, 8.f});
            circle.setFillColor(sf::Color::White);
        }
        _window.draw(circle);
    }
}