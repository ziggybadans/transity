#include "LineRenderSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "systems/gameplay/LineCreationSystem.h" // For ActiveLine
#include <algorithm>
#include <vector>
#include "core/Curve.h"

void LineRenderSystem::render(const entt::registry &registry, sf::RenderWindow &window,
                              const sf::View &view, const sf::Color& highlightColor) {
    // Render finalized lines
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        if (lineComp.curvePoints.size() < 2) continue;

        bool isSelected = registry.all_of<SelectedComponent>(entity);
        sf::Color lineColor = isSelected ? highlightColor : lineComp.color;
        float thickness = isSelected ? 16.0f : 8.0f;

        sf::VertexArray lineVertices(sf::PrimitiveType::TriangleStrip);

        for (size_t i = 0; i < lineComp.curvePoints.size(); ++i) {
            const auto& current_pos = lineComp.curvePoints[i];
            sf::Vector2f dir;

            if (i < lineComp.curvePoints.size() - 1) {
                dir = lineComp.curvePoints[i+1] - current_pos;
            } else {
                dir = current_pos - lineComp.curvePoints[i-1];
            }

            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len == 0) continue;
            sf::Vector2f perp(-dir.y / len, dir.x / len);
            
            size_t segmentIndex = lineComp.curveSegmentIndices[i];
            const sf::Vector2f offset = (segmentIndex < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[segmentIndex] : sf::Vector2f(0, 0);
            sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

            lineVertices.append({current_pos + offset - thicknessOffset, lineColor});
            lineVertices.append({current_pos + offset + thicknessOffset, lineColor});
        }
        window.draw(lineVertices);
    }

    // Render active line being created
    if (registry.ctx().contains<ActiveLine>()) {
        const auto& activeLine = registry.ctx().get<ActiveLine>();
        if (activeLine.points.empty()) return;

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);

        // Add this block
        if (registry.ctx().contains<LinePreview>()) {
            const auto& preview = registry.ctx().get<LinePreview>();
            if (preview.snapPosition) {
                mousePos = *preview.snapPosition;
            }
        }
        
        std::vector<sf::Vector2f> previewPoints;
        for(const auto& p : activeLine.points) {
            previewPoints.push_back(p.position);
        }
        previewPoints.push_back(mousePos);

        auto curveData = Curve::generateCatmullRom(previewPoints);

        sf::VertexArray lineVertices(sf::PrimitiveType::LineStrip);
        for(const auto& p : curveData.points) {
            lineVertices.append({p, sf::Color::Yellow});
        }
        window.draw(lineVertices);

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

        // ADDED: Draw control points of all existing lines
        sf::CircleShape existingControlPointMarker(6.f);
        existingControlPointMarker.setFillColor(sf::Color::Transparent);
        existingControlPointMarker.setOutlineColor(sf::Color::White);
        existingControlPointMarker.setOutlineThickness(1.f);
        existingControlPointMarker.setOrigin(sf::Vector2f(6.f, 6.f));

        auto allLinesView = registry.view<const LineComponent>();
        for (auto entity : allLinesView) {
            const auto& lineComp = allLinesView.get<const LineComponent>(entity);
            for (const auto& point : lineComp.points) {
                if (point.type == LinePointType::CONTROL_POINT) {
                    existingControlPointMarker.setPosition(point.position);
                    window.draw(existingControlPointMarker);
                }
            }
        }
    }
}