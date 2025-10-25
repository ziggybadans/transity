#include "PathRenderSystem.h"
#include "components/PassengerComponents.h"
#include "components/GameLogicComponents.h"
#include "components/LineComponents.h"
#include "components/RenderComponents.h"
#include "components/GameLogicComponents.h"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <optional>

PathRenderSystem::PathRenderSystem() {}

void PathRenderSystem::render(const entt::registry& registry, sf::RenderTarget& target) {
    auto view = registry.view<const VisualizePathComponent, const PathComponent, const PassengerComponent>();

    for (auto entity : view) {
        const auto& path = view.get<const PathComponent>(entity);
        const auto& passenger = view.get<const PassengerComponent>(entity);

        if (path.nodes.empty()) {
            continue;
        }

        std::optional<sf::Vector2f> currentPosition;
        if (registry.valid(passenger.currentContainer) && registry.all_of<PositionComponent>(passenger.currentContainer)) {
            currentPosition = registry.get<const PositionComponent>(passenger.currentContainer).coordinates;
        }

        if (!currentPosition.has_value()) {
            continue;
        }

        sf::VertexArray lines(sf::PrimitiveType::LineStrip);

        lines.append({*currentPosition, sf::Color::Yellow});

        for (size_t i = path.currentNodeIndex; i < path.nodes.size(); ++i) {
            entt::entity nodeEntity = path.nodes[i];
            if (!registry.valid(nodeEntity)) continue;

            if (const auto* stationPosition = registry.try_get<const PositionComponent>(nodeEntity)) {
                lines.append({{stationPosition->coordinates.x, stationPosition->coordinates.y}, sf::Color::Yellow});
            }
        }
        target.draw(lines);
    }
}