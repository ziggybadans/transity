#include "LineRenderSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "systems/gameplay/LineCreationSystem.h" // For ActiveLine
#include <algorithm>
#include <vector>

void LineRenderSystem::render(const entt::registry &registry, sf::RenderWindow &window,
                              const sf::View &view, const sf::Color& highlightColor) {
    // Render finalized lines
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        if (lineComp.points.size() < 2) continue;

        bool isSelected = registry.all_of<SelectedComponent>(entity);
        sf::Color lineColor = isSelected ? highlightColor : lineComp.color;
        float thickness = isSelected ? 16.0f : 8.0f;

        sf::VertexArray lineVertices(sf::PrimitiveType::TriangleStrip);

        for (size_t i = 0; i < lineComp.points.size(); ++i) {
            const auto& current_pos = lineComp.points[i].position;

            if (i == 0) { // First point
                const auto& next_pos = lineComp.points[i + 1].position;
                sf::Vector2f dir = next_pos - current_pos;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0) continue;
                sf::Vector2f perp(-dir.y / len, dir.x / len);
                
                const sf::Vector2f offset = (i < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[i] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

                lineVertices.append({current_pos + offset - thicknessOffset, lineColor});
                lineVertices.append({current_pos + offset + thicknessOffset, lineColor});
            } else if (i == lineComp.points.size() - 1) { // Last point
                const auto& prev_pos = lineComp.points[i - 1].position;
                sf::Vector2f dir = current_pos - prev_pos;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0) continue;
                sf::Vector2f perp(-dir.y / len, dir.x / len);

                const sf::Vector2f offset = ((i - 1) < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[i - 1] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

                lineVertices.append({current_pos + offset - thicknessOffset, lineColor});
                lineVertices.append({current_pos + offset + thicknessOffset, lineColor});
            } else { // Intermediate point (join)
                const auto& prev_pos = lineComp.points[i - 1].position;
                const auto& next_pos = lineComp.points[i + 1].position;

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

    // Render active line being created
    if (registry.ctx().contains<ActiveLine>()) {
        const auto& activeLine = registry.ctx().get<ActiveLine>();
        if (!activeLine.points.empty()) {
            // Draw segments between points
            for (size_t i = 0; i < activeLine.points.size() - 1; ++i) {
                const auto &pos1 = activeLine.points[i].position;
                const auto &pos2 = activeLine.points[i + 1].position;
                sf::Vertex line[] = {{pos1, sf::Color::Yellow}, {pos2, sf::Color::Yellow}};
                window.draw(line, 2, sf::PrimitiveType::Lines);
            }

            // Draw control point markers
            sf::CircleShape controlPointMarker(4.f);
            controlPointMarker.setFillColor(sf::Color::Yellow);
            controlPointMarker.setOrigin(sf::Vector2f(4.f, 4.f));
            for(const auto& point : activeLine.points) {
                if (point.type == LinePointType::CONTROL_POINT) {
                    controlPointMarker.setPosition(point.position);
                    window.draw(controlPointMarker);
                }
            }

            // Draw line from last point to mouse
            const auto &lastPos = activeLine.points.back().position;
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
            sf::Vertex lineToMouse[] = {{lastPos, sf::Color::Yellow}, {mousePos, sf::Color::Yellow}};
            window.draw(lineToMouse, 2, sf::PrimitiveType::Lines);
        }
    }
}