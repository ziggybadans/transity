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

        sf::VertexArray lineVertices(sf::PrimitiveType::TriangleStrip);

        for (size_t i = 0; i < lineComp.stops.size(); ++i) {
            const auto& current_pos = registry.get<const PositionComponent>(lineComp.stops[i]).coordinates;

            if (i == 0) { // First station
                if (!registry.valid(lineComp.stops[i+1])) continue;
                const auto& next_pos = registry.get<const PositionComponent>(lineComp.stops[i + 1]).coordinates;
                sf::Vector2f dir = next_pos - current_pos;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0) continue;
                sf::Vector2f perp(-dir.y / len, dir.x / len);
                
                const sf::Vector2f offset = (i < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[i] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

                lineVertices.append({current_pos + offset - thicknessOffset, lineColor});
                lineVertices.append({current_pos + offset + thicknessOffset, lineColor});
            } else if (i == lineComp.stops.size() - 1) { // Last station
                if (!registry.valid(lineComp.stops[i-1])) continue;
                const auto& prev_pos = registry.get<const PositionComponent>(lineComp.stops[i - 1]).coordinates;
                sf::Vector2f dir = current_pos - prev_pos;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0) continue;
                sf::Vector2f perp(-dir.y / len, dir.x / len);

                const sf::Vector2f offset = ((i - 1) < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[i - 1] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

                lineVertices.append({current_pos + offset - thicknessOffset, lineColor});
                lineVertices.append({current_pos + offset + thicknessOffset, lineColor});
            } else { // Intermediate station (join)
                if (!registry.valid(lineComp.stops[i-1]) || !registry.valid(lineComp.stops[i+1])) continue;
                const auto& prev_pos = registry.get<const PositionComponent>(lineComp.stops[i - 1]).coordinates;
                const auto& next_pos = registry.get<const PositionComponent>(lineComp.stops[i + 1]).coordinates;

                // End of incoming segment
                sf::Vector2f dir_in = current_pos - prev_pos;
                float len_in = std::sqrt(dir_in.x * dir_in.x + dir_in.y * dir_in.y);
                if (len_in == 0) continue;
                sf::Vector2f perp_in(-dir_in.y / len_in, dir_in.x / len_in);
                const sf::Vector2f offset_in = ((i - 1) < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[i - 1] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset_in = (thickness / 2.f) * perp_in;
                
                lineVertices.append({current_pos + offset_in - thicknessOffset_in, lineColor});
                lineVertices.append({current_pos + offset_in + thicknessOffset_in, lineColor});

                // Start of outgoing segment
                sf::Vector2f dir_out = next_pos - current_pos;
                float len_out = std::sqrt(dir_out.x * dir_out.x + dir_out.y * dir_out.y);
                if (len_out == 0) continue;
                sf::Vector2f perp_out(-dir_out.y / len_out, dir_out.x / len_out);
                const sf::Vector2f offset_out = (i < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[i] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset_out = (thickness / 2.f) * perp_out;

                lineVertices.append({current_pos + offset_out - thicknessOffset_out, lineColor});
                lineVertices.append({current_pos + offset_out + thicknessOffset_out, lineColor});
            }
        }
        window.draw(lineVertices);
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