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
        if (passenger.state == PassengerState::ON_TRAIN && passenger.currentTrain.has_value()) {
            entt::entity trainEntity = passenger.currentTrain.value();
            if (registry.valid(trainEntity) && registry.all_of<PositionComponent>(trainEntity)) {
                currentPosition = registry.get<const PositionComponent>(trainEntity).coordinates;
            }
        } else { // WAITING_FOR_TRAIN or ARRIVED
            if (path.currentNodeIndex < path.nodes.size()) {
                entt::entity currentStation = path.nodes[path.currentNodeIndex];
                if (registry.valid(currentStation) && registry.all_of<PositionComponent>(currentStation)) {
                    currentPosition = registry.get<const PositionComponent>(currentStation).coordinates;
                }
            }
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