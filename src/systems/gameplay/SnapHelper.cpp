#include "systems/gameplay/SnapHelper.h"
#include "components/LineComponents.h"
#include "Constants.h"
#include <cmath>

std::optional<SnapResult> SnapHelper::findSnap(
    entt::registry &registry, const sf::Vector2f &mousePos,
    std::optional<sf::Vector2f> previousPointPos,
    std::optional<std::pair<entt::entity, size_t>> ignorePoint) {

    const float SNAP_RADIUS_SQUARED = Constants::LINE_SNAP_RADIUS * Constants::LINE_SNAP_RADIUS;
    float closestDistSq = SNAP_RADIUS_SQUARED;
    std::optional<SnapInfo> snapInfo;

    // 1. Find the closest snap candidate (control point or city)
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        for (size_t i = 0; i < lineComp.points.size(); ++i) {
            if (ignorePoint && ignorePoint->first == entity && ignorePoint->second == i) {
                continue;
            }

            const auto &point = lineComp.points[i];
            if (point.type == LinePointType::CONTROL_POINT) {
                sf::Vector2f diff = mousePos - point.position;
                float distSq = diff.x * diff.x + diff.y * diff.y;
                if (distSq < closestDistSq) {
                    closestDistSq = distSq;
                    snapInfo = SnapInfo{entity, i};
                }
            }
        }
    }

    auto cityView = registry.view<const CityComponent, const PositionComponent>();
    for (auto entity : cityView) {
        const auto &pos = cityView.get<const PositionComponent>(entity);
        sf::Vector2f diff = mousePos - pos.coordinates;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            snapInfo = SnapInfo{entity, (size_t)-1};
        }
    }

    if (!snapInfo) {
        return std::nullopt;
    }

    // 2. If a candidate was found, calculate the detailed snap result
    SnapResult result;
    result.info = *snapInfo;
    result.side = 0.f;

    sf::Vector2f targetPointPos;
    sf::Vector2f tangent;
    bool tangentFound = false;

    if (result.info.snappedToPointIndex != (size_t)-1) { // Snapped to a control point
        const auto &targetLine = registry.get<LineComponent>(result.info.snappedToEntity);
        targetPointPos = targetLine.points[result.info.snappedToPointIndex].position;

        sf::Vector2f p_prev, p_next;
        if (result.info.snappedToPointIndex > 0) {
            p_prev = targetLine.points[result.info.snappedToPointIndex - 1].position;
        } else if (targetLine.points.size() > 1) {
            p_prev = targetPointPos - (targetLine.points[1].position - targetPointPos);
        }

        if (result.info.snappedToPointIndex < targetLine.points.size() - 1) {
            p_next = targetLine.points[result.info.snappedToPointIndex + 1].position;
        } else if (targetLine.points.size() > 1) {
            p_next = targetPointPos + (targetPointPos - targetLine.points[targetLine.points.size() - 2].position);
        }
        
        if (p_prev != p_next) {
            tangent = p_next - p_prev;
            tangentFound = true;
        }

    } else { // Snapped to a city
        targetPointPos = registry.get<PositionComponent>(result.info.snappedToEntity).coordinates;
        const auto& cityComp = registry.get<CityComponent>(result.info.snappedToEntity);

        if (previousPointPos && !cityComp.connectedLines.empty()) {
            sf::Vector2f incomingDir = targetPointPos - *previousPointPos;
            if (auto len = std::sqrt(incomingDir.x * incomingDir.x + incomingDir.y * incomingDir.y); len > 0) {
                incomingDir /= len;
            }

            float bestDot = -1.0f;
            for (entt::entity lineEntity : cityComp.connectedLines) {
                const auto& lineComp = registry.get<LineComponent>(lineEntity);
                for (size_t i = 0; i < lineComp.points.size(); ++i) {
                    if (lineComp.points[i].type == LinePointType::STOP && lineComp.points[i].stationEntity == result.info.snappedToEntity) {
                        sf::Vector2f p_prev, p_next;
                        if (i > 0) p_prev = lineComp.points[i - 1].position;
                        else if (lineComp.points.size() > 1) p_prev = lineComp.points[i].position - (lineComp.points[i + 1].position - lineComp.points[i].position);
                        
                        if (i < lineComp.points.size() - 1) p_next = lineComp.points[i + 1].position;
                        else if (lineComp.points.size() > 1) p_next = lineComp.points[i].position + (lineComp.points[i].position - lineComp.points[i - 1].position);

                        if (p_prev != p_next) {
                            sf::Vector2f currentTangent = p_next - p_prev;
                            if (auto len = std::sqrt(currentTangent.x * currentTangent.x + currentTangent.y * currentTangent.y); len > 0) {
                                currentTangent /= len;
                            }

                            float dot = std::abs(incomingDir.x * currentTangent.x + incomingDir.y * currentTangent.y);
                            if (dot > bestDot) {
                                bestDot = dot;
                                tangent = currentTangent;
                                tangentFound = true;
                            }
                        }
                        break;
                    }
                }
            }
        }
        
        if (!tangentFound && previousPointPos) {
            tangent = targetPointPos - *previousPointPos;
            tangentFound = true;
        }
    }

    result.position = targetPointPos;

    if (tangentFound) {
        float len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);
        if (len > 0) {
            tangent /= len;
            result.tangent = tangent;

            sf::Vector2f perpendicular = {-tangent.y, tangent.x};
            sf::Vector2f mouseVec = mousePos - targetPointPos;
            float perp_dist = mouseVec.x * perpendicular.x + mouseVec.y * perpendicular.y;

            if (std::abs(perp_dist) < Constants::LINE_CENTER_SNAP_RADIUS) {
                result.side = 0.f;
                result.position = targetPointPos;
            } else {
                result.side = (perp_dist > 0) ? 1.f : -1.f;
                sf::Vector2f sideOffset = perpendicular * result.side * Constants::LINE_PARALLEL_OFFSET;
                result.position = targetPointPos + sideOffset;
            }
        }
    }
    
    return result;
}