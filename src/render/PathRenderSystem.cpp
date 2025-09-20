#include "PathRenderSystem.h"
#include "components/PassengerComponents.h"
#include "components/GameLogicComponents.h"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

PathRenderSystem::PathRenderSystem() {}

void PathRenderSystem::render(const entt::registry& registry, sf::RenderWindow& window) {
    auto view = registry.view<const VisualizePathComponent, const PathComponent, const PositionComponent>();

    for (auto entity : view) {
        const auto& path = view.get<const PathComponent>(entity);
        const auto& passengerPosition = view.get<const PositionComponent>(entity);

        if (path.nodes.empty() || path.currentNodeIndex >= path.nodes.size()) {
            continue;
        }

        sf::VertexArray lines(sf::PrimitiveType::LineStrip);

        // Start the line from the passenger's current position.
        lines.append({{passengerPosition.coordinates.x, passengerPosition.coordinates.y}, sf::Color::Yellow});

        // Draw the rest of the path from the current node onwards.
        for (size_t i = path.currentNodeIndex; i < path.nodes.size(); ++i) {
            entt::entity nodeEntity = path.nodes[i];
            if (!registry.valid(nodeEntity)) continue;

            // A path node can be a station or a line. We only care about station positions.
            if (const auto* stationPosition = registry.try_get<const PositionComponent>(nodeEntity)) {
                lines.append({{stationPosition->coordinates.x, stationPosition->coordinates.y}, sf::Color::Yellow});
            }
        }
        window.draw(lines);
    }
}