#include "LineEditingRenderSystem.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
#include "components/LineComponents.h"
#include <SFML/Graphics/CircleShape.hpp>

LineEditingRenderSystem::LineEditingRenderSystem() = default;

LineEditingRenderSystem::~LineEditingRenderSystem() = default;

void LineEditingRenderSystem::draw(sf::RenderTarget& target, entt::registry& registry, GameState& gameState) {
    if (gameState.currentInteractionMode != InteractionMode::EDIT_LINE) {
        return;
    }

    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto& line = lineView.get<const LineComponent>(entity);
        bool isSelectedLine = gameState.selectedEntity.has_value() && gameState.selectedEntity.value() == entity;

        const LineEditingComponent* editingState = nullptr;
        if (isSelectedLine) {
            editingState = registry.try_get<LineEditingComponent>(entity);
        }

        for (size_t i = 0; i < line.points.size(); ++i) {
            const auto& point = line.points[i];
            sf::CircleShape circle;
            circle.setPosition(point.position);

            if (isSelectedLine && editingState && editingState->selectedPointIndex.has_value() && editingState->selectedPointIndex.value() == i) {
                circle.setRadius(10.f);
                circle.setOrigin({10.f, 10.f});
                circle.setFillColor(sf::Color::Red);
            } else {
                circle.setRadius(8.f);
                circle.setOrigin({8.f, 8.f});
                circle.setFillColor(sf::Color::White);
            }
            target.draw(circle);
        }
    }
}