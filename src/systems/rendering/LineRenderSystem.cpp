#include "LineRenderSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "systems/gameplay/LineCreationSystem.h" // For ActiveLine
#include "app/InteractionMode.h"
#include <algorithm>
#include <vector>
#include "core/Curve.h"

void createThickLine(sf::VertexArray& vertices, const std::vector<sf::Vector2f>& points, float thickness, sf::Color color) {
    if (points.size() < 2) {
        vertices.clear();
        return;
    }

    vertices.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
    vertices.resize(points.size() * 2);

    float halfThickness = thickness / 2.f;

    for (std::size_t i = 0; i < points.size(); ++i) {
        sf::Vector2f p1 = points[i];
        sf::Vector2f p0 = (i > 0) ? points[i - 1] : p1;
        sf::Vector2f p2 = (i < points.size() - 1) ? points[i + 1] : p1;

        sf::Vector2f dir1 = p1 - p0;
        float len1 = std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y);
        if (len1 != 0) dir1 /= len1;

        sf::Vector2f dir2 = p2 - p1;
        float len2 = std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y);
        if (len2 != 0) dir2 /= len2;

        sf::Vector2f normal;
        if (i == 0) {
            normal = sf::Vector2f(-dir2.y, dir2.x);
        } else if (i == points.size() - 1) {
            normal = sf::Vector2f(-dir1.y, dir1.x);
        } else {
            sf::Vector2f miter = dir1 + dir2;
            float miterLen = std::sqrt(miter.x * miter.x + miter.y * miter.y);
            if (miterLen > 1e-6) {
                normal = sf::Vector2f(-miter.y, miter.x) / miterLen;
            } else {
                normal = sf::Vector2f(-dir1.y, dir1.x);
            }
        }

        float miterRatio = 1.0f;
        if (i > 0 && i < points.size() - 1) {
            float dot = dir1.x * dir2.x + dir1.y * dir2.y;
            dot = std::max(-0.99f, std::min(0.99f, dot));
            miterRatio = std::sqrt(2.0f / (1.0f + dot));
        }
        
        // Clamp ratio to prevent extreme spikes on sharp angles
        if (miterRatio > 2.5f) miterRatio = 2.5f;

        vertices[i * 2].position = p1 - normal * (halfThickness * miterRatio);
        vertices[i * 2].color = color;
        vertices[i * 2 + 1].position = p1 + normal * (halfThickness * miterRatio);
        vertices[i * 2 + 1].color = color;
    }
}

void drawBarberPolePolyline(sf::RenderWindow& window, const std::vector<sf::Vector2f>& points, float thickness, const std::vector<sf::Color>& colors, float phaseOffset) {
    if (points.size() < 2 || colors.empty()) return;

    // Pre-calculate miter normals and lengths for each vertex
    std::vector<sf::Vector2f> miterNormals(points.size());
    std::vector<float> miterRatios(points.size(), 1.0f);
    float halfThickness = thickness / 2.f;

    for (size_t i = 0; i < points.size(); ++i) {
        sf::Vector2f p1 = points[i];
        sf::Vector2f p0 = (i > 0) ? points[i - 1] : p1;
        sf::Vector2f p2 = (i < points.size() - 1) ? points[i + 1] : p1;

        sf::Vector2f dir1 = p1 - p0;
        float len1 = std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y);
        if (len1 != 0) dir1 /= len1;

        sf::Vector2f dir2 = p2 - p1;
        float len2 = std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y);
        if (len2 != 0) dir2 /= len2;

        if (i == 0) {
            miterNormals[i] = sf::Vector2f(-dir2.y, dir2.x);
        } else if (i == points.size() - 1) {
            miterNormals[i] = sf::Vector2f(-dir1.y, dir1.x);
        } else {
            sf::Vector2f miter = dir1 + dir2;
            float miterLen = std::sqrt(miter.x * miter.x + miter.y * miter.y);
            if (miterLen > 1e-6) {
                miterNormals[i] = sf::Vector2f(-miter.y, miter.x) / miterLen;
            } else {
                miterNormals[i] = sf::Vector2f(-dir1.y, dir1.x);
            }
            
            float dot = dir1.x * dir2.x + dir1.y * dir2.y;
            dot = std::max(-0.99f, std::min(0.99f, dot));
            miterRatios[i] = std::min(2.5f, std::sqrt(2.0f / (1.0f + dot)));
        }
    }

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
        
        sf::Vector2f thicknessOffset1 = miterNormals[i] * (halfThickness * miterRatios[i]);
        sf::Vector2f thicknessOffset2 = miterNormals[i+1] * (halfThickness * miterRatios[i+1]);

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
                float t1 = (clampedStripeStartDist - startDist) / segmentLen;
                float t2 = (clampedStripeEndDist - startDist) / segmentLen;

                sf::Vector2f stripe_p1 = p1 + dir * (clampedStripeStartDist - startDist);
                sf::Vector2f stripe_p2 = p1 + dir * (clampedStripeEndDist - startDist);

                // Interpolate the thickness offset along the segment
                sf::Vector2f offset1 = thicknessOffset1 * (1.f - t1) + thicknessOffset2 * t1;
                sf::Vector2f offset2 = thicknessOffset1 * (1.f - t2) + thicknessOffset2 * t2;

                sf::VertexArray stripe(sf::PrimitiveType::TriangleStrip, 4);
                stripe[0].position = stripe_p1 - offset1;
                stripe[1].position = stripe_p1 + offset1;
                stripe[2].position = stripe_p2 - offset2;
                stripe[3].position = stripe_p2 + offset2;

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
                // This is a non-shared segment. Collect all points for the continuous non-shared curve.
                size_t endPointIndex = i;
                while (endPointIndex < lineComp.curvePoints.size() - 1) {
                    size_t currentSegmentIndex = lineComp.curveSegmentIndices[endPointIndex];
                    auto currentIt = lineComp.sharedSegments.find(currentSegmentIndex);
                    if (currentIt != lineComp.sharedSegments.end() && currentIt->second->lines.size() > 1) {
                        break; // Stop before this shared segment
                    }
                    endPointIndex++;
                }

                // The polyline consists of points from `i` to `endPointIndex`.
                std::vector<sf::Vector2f> offsetPolyline;
                for (size_t k = i; k <= endPointIndex; ++k) {
                    size_t segIdx = (k < lineComp.curvePoints.size() - 1) ? lineComp.curveSegmentIndices[k] : lineComp.curveSegmentIndices[k - 1];
                    const sf::Vector2f offset = (segIdx < lineComp.pathOffsets.size()) ? lineComp.pathOffsets[segIdx] : sf::Vector2f(0, 0);
                    offsetPolyline.push_back(lineComp.curvePoints[k] + offset);
                }

                if (offsetPolyline.size() >= 2) {
                    sf::Color lineColor = isSelected ? highlightColor : lineComp.color;
                    sf::VertexArray lineVertices;
                    createThickLine(lineVertices, offsetPolyline, thickness, lineColor);
                    window.draw(lineVertices);
                }
                
                i = endPointIndex;
                if (i == 0) i++; // Ensure progress if the first segment is the only one processed
            }

        }
    }

    // Render active line being created
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
            window.draw(lineVertices);
        }
    }

    if (registry.ctx().contains<ActiveLine>()) {
        const auto& activeLine = registry.ctx().get<ActiveLine>();
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