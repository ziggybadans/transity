#include "systems/rendering/LineRenderSystem.h"
#include "render/LineDrawer.h"
#include "components/GameLogicComponents.h"
#include "components/LineComponents.h"
#include "systems/gameplay/LineCreationSystem.h"
#include "app/InteractionMode.h"
#include <vector>

void LineRenderSystem::render(const entt::registry &registry, sf::RenderTarget &target, const GameState& gameState,
                              const sf::View &view, const sf::Color& highlightColor) {
    renderFinalizedLines(registry, target, highlightColor);

    if (gameState.currentInteractionMode == InteractionMode::CREATE_LINE) {
        renderActiveLinePreview(registry, target);
        renderSnappingIndicators(registry, target);
    }
}

void LineRenderSystem::renderFinalizedLines(const entt::registry &registry, sf::RenderTarget &target, const sf::Color& highlightColor) {
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        if (lineComp.curvePoints.size() < 2) continue;

        bool isSelected = registry.all_of<SelectedComponent>(entity);
        float thickness = isSelected ? 16.0f : 8.0f;

        for (size_t i = 0; i < lineComp.curvePoints.size() - 1; ) {
            size_t segmentIndex = lineComp.curveSegmentIndices[i];
            auto it = lineComp.sharedSegments.find(segmentIndex);

            if (it != lineComp.sharedSegments.end() && it->second->lines.size() > 1) {
                std::vector<sf::Color> colors;
                for (entt::entity line_entity : it->second->lines) {
                    colors.push_back(registry.get<LineComponent>(line_entity).color);
                }

                int lineIndex = -1;
                for (int k = 0; k < it->second->lines.size(); ++k) {
                    if (it->second->lines[k] == entity) {
                        lineIndex = k;
                        break;
                    }
                }

                float phaseOffset = (lineIndex != -1) ? (10.0f / it->second->lines.size()) * lineIndex : 0.0f;

                std::vector<sf::Vector2f> polyline;
                polyline.push_back(lineComp.curvePoints[i]);
                size_t j = i;
                while (j < lineComp.curvePoints.size() - 1 && lineComp.curveSegmentIndices[j] == segmentIndex) {
                    polyline.push_back(lineComp.curvePoints[j + 1]);
                    j++;
                }

                LineDrawer::drawBarberPolePolyline(target, polyline, thickness, colors, phaseOffset);
                i = j;
            } else {
                size_t endPointIndex = i;
                while (endPointIndex < lineComp.curvePoints.size() - 1) {
                    size_t currentSegmentIndex = lineComp.curveSegmentIndices[endPointIndex];
                    auto currentIt = lineComp.sharedSegments.find(currentSegmentIndex);
                    if (currentIt != lineComp.sharedSegments.end() && currentIt->second->lines.size() > 1) {
                        break;
                    }
                    endPointIndex++;
                }

                std::vector<sf::Vector2f> offsetPolyline;
                for (size_t k = i; k <= endPointIndex; ++k) {
                    size_t segIdx = (k < lineComp.curvePoints.size() - 1) ? lineComp.curveSegmentIndices[k] : lineComp.curveSegmentIndices[k - 1];
                    const sf::Vector2f offset = (segIdx < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[segIdx] : sf::Vector2f(0, 0);
                    offsetPolyline.push_back(lineComp.curvePoints[k] + offset);
                }

                if (offsetPolyline.size() >= 2) {
                    sf::Color lineColor = isSelected ? highlightColor : lineComp.color;
                    sf::VertexArray lineVertices;
                    LineDrawer::createThickLine(lineVertices, offsetPolyline, thickness, lineColor);
                    target.draw(lineVertices);
                }
                
                i = endPointIndex;
                if (i == 0) i++;
            }
        }
    }
}

void LineRenderSystem::renderActiveLinePreview(const entt::registry &registry, sf::RenderTarget &target) {
    if (registry.ctx().contains<LinePreview>()) {
        const auto& preview = registry.ctx().get<LinePreview>();
        if (preview.curvePoints.size() >= 2) {
            sf::VertexArray lineVertices(sf::PrimitiveType::LineStrip);
            for (size_t i = 0; i < preview.curvePoints.size() - 1; ++i) {
                sf::Color segmentColor = (i < preview.validSegments.size() && preview.validSegments[i])
                                             ? sf::Color::Yellow
                                             : sf::Color::Red;
                lineVertices.append({preview.curvePoints[i], segmentColor});
                lineVertices.append({preview.curvePoints[i+1], segmentColor});
            }
            target.draw(lineVertices);
        }
    }
}

void LineRenderSystem::renderSnappingIndicators(const entt::registry &registry, sf::RenderTarget &target) {
    if (registry.ctx().contains<ActiveLine>()) {
        const auto& activeLine = registry.ctx().get<ActiveLine>();
        sf::CircleShape controlPointMarker(4.f, 30);
        controlPointMarker.setFillColor(sf::Color::Yellow);
        controlPointMarker.setOrigin({4.f, 4.f});
        for(const auto& point : activeLine.points) {
            if (point.type == LinePointType::CONTROL_POINT) {
                controlPointMarker.setPosition(point.position);
                target.draw(controlPointMarker);
            }
        }
    }

    sf::CircleShape existingControlPointMarker(6.f, 30);
    existingControlPointMarker.setFillColor(sf::Color::Transparent);
    existingControlPointMarker.setOutlineColor(sf::Color::White);
    existingControlPointMarker.setOutlineThickness(1.f);
    existingControlPointMarker.setOrigin({6.f, 6.f});

    auto allLinesView = registry.view<const LineComponent>();
    for (auto entity : allLinesView) {
        const auto& lineComp = allLinesView.get<const LineComponent>(entity);
        for (const auto& point : lineComp.points) {
            if (point.type == LinePointType::CONTROL_POINT) {
                existingControlPointMarker.setPosition(point.position);
                target.draw(existingControlPointMarker);
            }
        }
    }

    auto cityView = registry.view<const CityComponent, const PositionComponent>();
    for (auto entity : cityView) {
        const auto& pos = cityView.get<const PositionComponent>(entity);
        existingControlPointMarker.setPosition(pos.coordinates);
        target.draw(existingControlPointMarker);
    }

    if (registry.ctx().contains<LinePreview>()) {
        const auto& preview = registry.ctx().get<LinePreview>();
        if (preview.snapInfo && preview.snapTangent) {
            sf::Vector2f targetPos;
            if (preview.snapInfo->snappedToPointIndex != (size_t)-1) {
                const auto& line = registry.get<LineComponent>(preview.snapInfo->snappedToEntity);
                targetPos = line.points[preview.snapInfo->snappedToPointIndex].position;
            } else {
                targetPos = registry.get<PositionComponent>(preview.snapInfo->snappedToEntity).coordinates;
            }

            if (preview.snapSide != 0.f) {
                sf::VertexArray halfCircle(sf::PrimitiveType::TriangleFan);
                halfCircle.append({targetPos, sf::Color(255, 255, 255, 100)});
                sf::Vector2f perpendicular = {-(*preview.snapTangent).y, (*preview.snapTangent).x};
                perpendicular *= preview.snapSide;
                const int segments = 10;
                const float radius = 6.f;
                float startAngle = std::atan2(perpendicular.y, perpendicular.x) - (3.14159265f / 2.f);
                for (int i = 0; i <= segments; ++i) {
                    float angle = startAngle + (3.14159265f * i / segments);
                    sf::Vector2f pointPos = targetPos + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
                    halfCircle.append({pointPos, sf::Color(255, 255, 255, 100)});
                }
                target.draw(halfCircle);
            } else {
                sf::CircleShape centerSnapIndicator(6.f);
                centerSnapIndicator.setFillColor(sf::Color(255, 255, 255, 100));
                centerSnapIndicator.setOrigin({6.f, 6.f});
                centerSnapIndicator.setPosition(targetPos);
                target.draw(centerSnapIndicator);
            }
        }
    }
}