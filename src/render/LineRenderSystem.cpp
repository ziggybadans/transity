#include "LineRenderSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include <algorithm>
#include <vector>

void LineRenderSystem::render(const entt::registry &registry, sf::RenderWindow &window,
                              const sf::View &view, const sf::Color& highlightColor) {
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        if (lineComp.stops.size() < 2) continue;

        bool isSelected = registry.all_of<SelectedComponent>(entity);
        sf::Color lineColor = isSelected ? highlightColor : lineComp.color;
        float thickness = isSelected ? 16.0f : 8.0f;

        for (size_t i = 0; i < lineComp.stops.size() - 1; ++i) {
            if (!registry.valid(lineComp.stops[i]) || !registry.valid(lineComp.stops[i + 1]))
                continue;
            
            const auto &pos1 = registry.get<const PositionComponent>(lineComp.stops[i]).coordinates;
            const auto &pos2 = registry.get<const PositionComponent>(lineComp.stops[i + 1]).coordinates;

            sf::Vector2f direction = pos2 - pos1;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (length == 0) continue;
            sf::Vector2f unitDirection = direction / length;
            sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

            sf::Vector2f offset = (thickness / 2.f) * unitPerpendicular;

            sf::VertexArray quad(sf::PrimitiveType::Triangles, 6);
            quad[0].position = pos1 - offset;
            quad[1].position = pos2 - offset;
            quad[2].position = pos2 + offset;
            
            quad[3].position = pos2 + offset;
            quad[4].position = pos1 + offset;
            quad[5].position = pos1 - offset;

            for(int v = 0; v < 6; ++v) {
                quad[v].color = lineColor;
            }

            window.draw(quad);
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