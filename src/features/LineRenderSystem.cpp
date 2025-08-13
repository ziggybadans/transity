#include "LineRenderSystem.h"
#include "../Logger.h"
#include "../core/Constants.h"
#include "../core/GameLogicComponents.h"
#include <algorithm>
#include <vector>

void LineRenderSystem::render(const entt::registry &registry, sf::RenderWindow &window,
                              const sf::View &view) {
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        if (lineComp.stops.size() < 2) continue;

        for (size_t i = 0; i < lineComp.stops.size() - 1; ++i) {
            if (!registry.valid(lineComp.stops[i]) || !registry.valid(lineComp.stops[i + 1]))
                continue;
            const auto &pos1 = registry.get<const PositionComponent>(lineComp.stops[i]).coordinates;
            const auto &pos2 =
                registry.get<const PositionComponent>(lineComp.stops[i + 1]).coordinates;
            sf::Vertex line[] = {{pos1, lineComp.color, sf::Vector2f()},
                                 {pos2, lineComp.color, sf::Vector2f()}};
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }

    m_taggedStationsPairs.clear();
    auto activeStationsView = registry.view<const PositionComponent, const ActiveLineStationTag>();
    for (auto entity : activeStationsView) {
        m_taggedStationsPairs.push_back(
            {activeStationsView.get<const ActiveLineStationTag>(entity).order.value, entity});
    }

    if (!m_taggedStationsPairs.empty()) {
        std::sort(m_taggedStationsPairs.begin(), m_taggedStationsPairs.end());

        m_activeLineStations.clear();
        for (const auto &pair : m_taggedStationsPairs)
            m_activeLineStations.push_back(pair.second);

        for (size_t i = 0; i < m_activeLineStations.size() - 1; ++i) {
            const auto &pos1 =
                registry.get<const PositionComponent>(m_activeLineStations[i]).coordinates;
            const auto &pos2 =
                registry.get<const PositionComponent>(m_activeLineStations[i + 1]).coordinates;
            sf::Vertex line[] = {{pos1, sf::Color::Yellow, sf::Vector2f()},
                                 {pos2, sf::Color::Yellow, sf::Vector2f()}};
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }

        const auto &lastPos =
            registry.get<const PositionComponent>(m_activeLineStations.back()).coordinates;
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
        sf::Vertex lineToMouse[] = {{lastPos, sf::Color::Yellow, sf::Vector2f()},
                                    {mousePos, sf::Color::Yellow, sf::Vector2f()}};
        window.draw(lineToMouse, 2, sf::PrimitiveType::Lines);
    }
}