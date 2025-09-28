#include "LineRenderSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "systems/gameplay/LineCreationSystem.h" // For ActiveLine
#include "app/InteractionMode.h"
#include <algorithm>
#include <vector>
#include "core/Curve.h"

void drawBarberPolePolyline(sf::RenderWindow& window, const std::vector<sf::Vector2f>& points, float thickness, const std::vector<sf::Color>& colors, float phaseOffset) {
    if (points.size() < 2 || colors.empty()) return;

    float totalLength = 0.f;
    std::vector<float> segmentLengths;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        sf::Vector2f dir = points[i+1] - points[i];
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        segmentLengths.push_back(len);
        totalLength += len;
    }

    if (totalLength == 0.f) return;

    const float stripeLength = 10.0f;
    const float animationOffset = phaseOffset;

    float distanceAlongPath = 0.f;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        const sf::Vector2f& p1 = points[i];
        const sf::Vector2f& p2 = points[i+1];
        float segmentLen = segmentLengths[i];
        if (segmentLen == 0) continue;

        sf::Vector2f dir = (p2 - p1) / segmentLen;
        sf::Vector2f perp(-dir.y, dir.x);
        sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;

        float startDist = distanceAlongPath;
        float endDist = startDist + segmentLen;

        float currentStripeStart = std::ceil((startDist - animationOffset) / stripeLength) * stripeLength + animationOffset;

        while (currentStripeStart < endDist) {
            float stripeEnd = currentStripeStart + stripeLength;

            int colorIndex = static_cast<int>((currentStripeStart - animationOffset) / stripeLength);
            sf::Color color = colors[std::abs(colorIndex) % colors.size()];

            float clampedStripeStartDist = std::max(startDist, currentStripeStart);
            float clampedStripeEndDist = std::min(endDist, stripeEnd);

            if (clampedStripeStartDist < clampedStripeEndDist) {
                sf::Vector2f stripe_p1 = p1 + dir * (clampedStripeStartDist - startDist);
                sf::Vector2f stripe_p2 = p1 + dir * (clampedStripeEndDist - startDist);

                sf::VertexArray stripe(sf::PrimitiveType::TriangleStrip, 4);
                stripe[0].position = stripe_p1 - thicknessOffset;
                stripe[1].position = stripe_p1 + thicknessOffset;
                stripe[2].position = stripe_p2 - thicknessOffset;
                stripe[3].position = stripe_p2 + thicknessOffset;

                stripe[0].color = color;
                stripe[1].color = color;
                stripe[2].color = color;
                stripe[3].color = color;

                window.draw(stripe);
            }
            currentStripeStart += stripeLength;
        }
        distanceAlongPath += segmentLen;
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
        float thickness = isSelected ? 16.0f : 8.0f;

        // Segment-by-segment rendering
        for (size_t i = 0; i < lineComp.curvePoints.size() - 1; ) {
            size_t segmentIndex = lineComp.curveSegmentIndices[i];
            
            auto it = lineComp.sharedSegments.find(segmentIndex);
            if (it != lineComp.sharedSegments.end() && it->second->lines.size() > 1) {
                // This is a shared segment, draw with barber pole effect
                std::vector<sf::Color> colors;
                for (entt::entity line_entity : it->second->lines) {
                    colors.push_back(registry.get<LineComponent>(line_entity).color);
                }

                // Find the index of the current line in the shared segment
                int lineIndex = -1;
                for (int k = 0; k < it->second->lines.size(); ++k) {
                    if (it->second->lines[k] == entity) {
                        lineIndex = k;
                        break;
                    }
                }

                // Calculate phase offset
                float phaseOffset = 0.0f;
                if (lineIndex != -1) {
                    phaseOffset = (10.0f / it->second->lines.size()) * lineIndex;
                }

                // Collect all points for this continuous curve
                std::vector<sf::Vector2f> polyline;
                polyline.push_back(lineComp.curvePoints[i]);
                size_t j = i;
                while (j < lineComp.curvePoints.size() - 1 && lineComp.curveSegmentIndices[j] == segmentIndex) {
                    polyline.push_back(lineComp.curvePoints[j + 1]);
                    j++;
                }

                drawBarberPolePolyline(window, polyline, thickness, colors, phaseOffset);
                
                i = j; // Advance i past this curve
            } else {
                // Not a shared segment, draw normally
                const auto& p1 = lineComp.curvePoints[i];
                const auto& p2 = lineComp.curvePoints[i+1];

                sf::VertexArray lineSegment(sf::PrimitiveType::TriangleStrip);
                
                sf::Vector2f dir = p2 - p1;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len == 0) { i++; continue; }
                sf::Vector2f perp(-dir.y / len, dir.x / len);
                
                const sf::Vector2f offset = (segmentIndex < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[segmentIndex] : sf::Vector2f(0, 0);
                sf::Vector2f thicknessOffset = (thickness / 2.f) * perp;
                sf::Color lineColor = isSelected ? highlightColor : lineComp.color;

                lineSegment.append({p1 + offset - thicknessOffset, lineColor});
                lineSegment.append({p1 + offset + thicknessOffset, lineColor});
                lineSegment.append({p2 + offset - thicknessOffset, lineColor});
                lineSegment.append({p2 + offset + thicknessOffset, lineColor});
                
                window.draw(lineSegment);
                i++;
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

            auto curveData = Curve::generateMetroCurve(previewPoints, Constants::METRO_CURVE_RADIUS);

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