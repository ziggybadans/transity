#include "PathRenderSystem.h"
#include "components/PassengerComponents.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <optional>

PathRenderSystem::PathRenderSystem() {}

void PathRenderSystem::render(const entt::registry& registry, sf::RenderWindow& window) {
    auto view = registry.view<const VisualizePathComponent, const PathComponent, const PassengerComponent>();

    for (auto entity : view) {
        const auto& path = view.get<const PathComponent>(entity);
        const auto& passenger = view.get<const PassengerComponent>(entity);

        if (path.nodes.empty()) {
            continue;
        }

        // Determine the passenger's current position
        std::optional<sf::Vector2f> currentPosition;
        if (registry.valid(passenger.currentContainer) && registry.all_of<PositionComponent>(passenger.currentContainer)) {
            currentPosition = registry.get<const PositionComponent>(passenger.currentContainer).coordinates;
        }

        if (!currentPosition.has_value()) {
            continue;
        }

        sf::VertexArray lines(sf::PrimitiveType::LineStrip);

        // Start the line from the passenger's current position.
        lines.append({*currentPosition, sf::Color::Yellow});

        // Draw the rest of the path from the current node onwards.
        for (size_t i = path.currentNodeIndex; i < path.nodes.size(); ++i) {
            entt::entity nodeEntity = path.nodes[i];
            if (!registry.valid(nodeEntity)) continue;

            if (const auto* stationPosition = registry.try_get<const PositionComponent>(nodeEntity)) {
                lines.append({{stationPosition->coordinates.x, stationPosition->coordinates.y}, sf::Color::Yellow});
            }
        }
        window.draw(lines);
    }
}