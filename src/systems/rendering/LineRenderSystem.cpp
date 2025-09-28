#include "LineRenderSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "systems/gameplay/LineCreationSystem.h" // For ActiveLine
#include "app/InteractionMode.h"
#include <algorithm>
#include <vector>
#include "core/Curve.h"

void drawBarberPoleSegment(sf::RenderWindow& window, const sf::Vector2f& p1, const sf::Vector2f& p2, float thickness, const std::vector<sf::Color>& colors) {
    if (colors.empty()) return;

    sf::Vector2f dir = p2 - p1;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0) return;
    sf::Vector2f perp(-dir.y / len, dir.x / len);
    sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

    const float stripeLength = 10.0f; // Length of each color stripe
    int numStripes = std::max(1, static_cast<int>(len / stripeLength));
    
    for (int i = 0; i < numStripes; ++i) {
        sf::Vector2f start = p1 + dir * (static_cast<float>(i) / numStripes);
        sf::Vector2f end = p1 + dir * (static_cast<float>(i + 1) / numStripes);
        
        sf::VertexArray stripe(sf::PrimitiveType::TriangleStrip, 4);
        stripe[0].position = start - thicknessOffset;
        stripe[1].position = start + thicknessOffset;
        stripe[2].position = end - thicknessOffset;
        stripe[3].position = end + thicknessOffset;

        sf::Color color = colors[i % colors.size()];
        stripe[0].color = color;
        stripe[1].color = color;
        stripe[2].color = color;
        stripe[3].color = color;

        window.draw(stripe);
    }
}

void LineRenderSystem::render(const entt::registry &registry, sf::RenderWindow &window, const GameState& gameState,
                              const sf::View &view, const sf::Color& highlightColor) {
    // Render finalized lines
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        if (lineComp.curvePoints.size() < 2) continue;

        bool isSelected = registry.all_of<SelectedComponent>(entity);
        sf::Color lineColor = isSelected ? highlightColor : lineComp.color;
        float thickness = isSelected ? 16.0f : 8.0f;

        // Segment-by-segment rendering
        for (size_t i = 0; i < lineComp.curvePoints.size() - 1; ++i) {
            size_t segmentIndex = lineComp.curveSegmentIndices[i];
            const auto& p1 = lineComp.curvePoints[i];
            const auto& p2 = lineComp.curvePoints[i+1];
            
            auto it = lineComp.sharedSegments.find(segmentIndex);
            if (it != lineComp.sharedSegments.end() && it->second->lines.size() > 1) {
                // This is a shared segment, draw with barber pole effect
                std::vector<sf::Color> colors;
                for (entt::entity line_entity : it->second->lines) {
                    colors.push_back(registry.get<LineComponent>(line_entity).color);
                }
                drawBarberPoleSegment(window, p1, p2, thickness, colors);
            } else {
                // Not a shared segment, draw normally
                sf::VertexArray lineSegment(sf::PrimitiveType::TriangleStrip);
                
                sf::Vector2f dir = p2 - p1;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0) continue;
                sf::Vector2f perp(-dir.y / len, dir.x / len);
                
                const sf::Vector2f offset = (segmentIndex < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[segmentIndex] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

                lineSegment.append({p1 + offset - thicknessOffset, lineColor});
                lineSegment.append({p1 + offset + thicknessOffset, lineColor});
                lineSegment.append({p2 + offset - thicknessOffset, lineColor});
                lineSegment.append({p2 + offset + thicknessOffset, lineColor});
                
                window.draw(lineSegment);
            }
        }
    }

    // Render active line being created
    if (registry.ctx().contains<ActiveLine>()) {
        const auto& activeLine = registry.ctx().get<ActiveLine>();
        if (!activeLine.points.empty()) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);

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

            // Draw control point markers for the active line
            sf::CircleShape controlPointMarker(4.f);
            controlPointMarker.setFillColor(sf::Color::Yellow);
            controlPointMarker.setOrigin(sf::Vector2f(4.f, 4.f));
            for(const auto& point : activeLine.points) {
                if (point.type == LinePointType::CONTROL_POINT) {
                    controlPointMarker.setPosition(point.position);
                    window.draw(controlPointMarker);
                }
            }
        }
    }

    // If in line creation mode, draw all snappable control points
    if (gameState.currentInteractionMode == InteractionMode::CREATE_LINE) {
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

        auto cityView = registry.view<const CityComponent, const PositionComponent>();
        for (auto entity : cityView) {
            const auto& pos = cityView.get<const PositionComponent>(entity);
            existingControlPointMarker.setPosition(pos.coordinates);
            window.draw(existingControlPointMarker);
        }

        if (registry.ctx().contains<LinePreview>()) {
            const auto& preview = registry.ctx().get<LinePreview>();
            if (preview.snapInfo && preview.snapTangent) {
                sf::Vector2f targetPos;
                if (preview.snapInfo->snappedToPointIndex != (size_t)-1) { // Snapped to line
                    const auto& line = registry.get<LineComponent>(preview.snapInfo->snappedToEntity);
                    targetPos = line.points[preview.snapInfo->snappedToPointIndex].position;
                } else { // Snapped to city
                    targetPos = registry.get<PositionComponent>(preview.snapInfo->snappedToEntity).coordinates;
                }

                if (preview.snapSide != 0.f) {
                    // Draw half circle for side snap
                    sf::VertexArray halfCircle(sf::PrimitiveType::TriangleFan);
                    halfCircle.append({targetPos, sf::Color(255, 255, 255, 100)}); // Center point

                    sf::Vector2f perpendicular = {-(*preview.snapTangent).y, (*preview.snapTangent).x};
                    perpendicular *= preview.snapSide; // Point to the correct side

                    const int segments = 10;
                    const float radius = 6.f; // Match the control point marker radius
                    float startAngle = std::atan2(perpendicular.y, perpendicular.x) - (3.14159265f / 2.f);

                    for (int i = 0; i <= segments; ++i) {
                        float angle = startAngle + (3.14159265f * i / segments);
                        sf::Vector2f pointPos = targetPos + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
                        halfCircle.append({pointPos, sf::Color(255, 255, 255, 100)});
                    }
                    window.draw(halfCircle);
                } else {
                    // Draw full circle for center snap
                    sf::CircleShape centerSnapIndicator(6.f);
                    centerSnapIndicator.setFillColor(sf::Color(255, 255, 255, 100));
                    centerSnapIndicator.setOrigin({6.f, 6.f});
                    centerSnapIndicator.setPosition(targetPos);
                    window.draw(centerSnapIndicator);
                }
            }
        }
    }
}