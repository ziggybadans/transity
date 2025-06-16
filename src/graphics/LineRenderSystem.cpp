#include "LineRenderSystem.h"
#include "../core/Components.h"
#include "../Logger.h"
#include "../core/Constants.h"
#include <vector>
#include <algorithm>

void LineRenderSystem::render(entt::registry& registry, sf::RenderWindow& window, const sf::View& view) {
    auto lineView = registry.view<LineComponent>();
    for (auto entity : lineView) {
        const auto& lineComp = lineView.get<LineComponent>(entity);
        if (lineComp.stops.size() < 2) continue;

        for (size_t i = 0; i < lineComp.stops.size() - 1; ++i) {
            if (!registry.valid(lineComp.stops[i]) || !registry.valid(lineComp.stops[i+1])) continue;
            const auto& pos1 = registry.get<PositionComponent>(lineComp.stops[i]).coordinates;
            const auto& pos2 = registry.get<PositionComponent>(lineComp.stops[i+1]).coordinates;
            sf::Vertex line[] = { {pos1, lineComp.color, sf::Vector2f()}, {pos2, lineComp.color, sf::Vector2f()} };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }

    std::vector<std::pair<int, entt::entity>> taggedStationsPairs;
    auto activeStationsView = registry.view<PositionComponent, ActiveLineStationTag>();
    for (auto entity : activeStationsView) {
        taggedStationsPairs.push_back({activeStationsView.get<ActiveLineStationTag>(entity).order, entity});
    }

    if (!taggedStationsPairs.empty()) {
        std::sort(taggedStationsPairs.begin(), taggedStationsPairs.end());
        
        std::vector<entt::entity> activeLineStations;
        for(const auto& pair : taggedStationsPairs) activeLineStations.push_back(pair.second);

        for (size_t i = 0; i < activeLineStations.size() - 1; ++i) {
            const auto& pos1 = registry.get<PositionComponent>(activeLineStations[i]).coordinates;
            const auto& pos2 = registry.get<PositionComponent>(activeLineStations[i+1]).coordinates;
            sf::Vertex line[] = { {pos1, sf::Color::Yellow, sf::Vector2f()}, {pos2, sf::Color::Yellow, sf::Vector2f()} };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }

        const auto& lastPos = registry.get<PositionComponent>(activeLineStations.back()).coordinates;
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
        sf::Vertex lineToMouse[] = { {lastPos, sf::Color::Yellow, sf::Vector2f()}, {mousePos, sf::Color::Yellow, sf::Vector2f()} };
        window.draw(lineToMouse, 2, sf::PrimitiveType::Lines);
    }
}