#include "TrainRenderSystem.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
#include <SFML/Graphics/CircleShape.hpp>

TrainRenderSystem::TrainRenderSystem() {}

void TrainRenderSystem::render(const entt::registry &registry, sf::RenderWindow &window) {
    auto view = registry.view<const PositionComponent, const RenderableComponent, const TrainComponent>();
    for (auto entity : view) {
        const auto &pos = view.get<const PositionComponent>(entity);
        const auto &renderable = view.get<const RenderableComponent>(entity);

        sf::CircleShape shape(renderable.radius.value);
        shape.setFillColor(renderable.color);
        shape.setOrigin({renderable.radius.value, renderable.radius.value});
        shape.setPosition(pos.coordinates);
        
        window.draw(shape);
    }
}