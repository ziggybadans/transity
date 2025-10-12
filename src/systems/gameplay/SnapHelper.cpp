#include "systems/gameplay/SnapHelper.h"
#include "components/LineComponents.h"
#include "Constants.h"
#include <cmath>

std::optional<SnapResult> SnapHelper::findSnap(
    entt::registry &registry, const sf::Vector2f &mousePos,
    std::optional<std::pair<entt::entity, size_t>> ignorePoint) {

    const float SNAP_RADIUS_SQUARED = Constants::LINE_SNAP_RADIUS * Constants::LINE_SNAP_RADIUS;
    float closestDistSq = SNAP_RADIUS_SQUARED;
    std::optional<SnapInfo> snapInfo;

    // 1. Find the closest snap candidate (control point or city)
    // Check control points on existing lines
    auto lineView = registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto &lineComp = lineView.get<const LineComponent>(entity);
        for (size_t i = 0; i < lineComp.points.size(); ++i) {
            // Ignore the point being dragged, if specified
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

    // Check cities
    auto cityView = registry.view<const CityComponent, const PositionComponent>();
    for (auto entity : cityView) {
        const auto &pos = cityView.get<const PositionComponent>(entity);
        sf::Vector2f diff = mousePos - pos.coordinates;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            snapInfo = SnapInfo{entity, (size_t)-1}; // Use -1 to indicate a city
        }
    }

    // 2. If a candidate was found, calculate the detailed snap result
    if (!snapInfo) {
        return std::nullopt;
    }

    SnapResult result;
    result.info = *snapInfo;
    result.side = 0.f;

    sf::Vector2f targetPointPos;
    sf::Vector2f tangent;
    bool tangentFound = false;

    // Determine tangent based on snap type
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
        // For a generic helper, we snap to the city center without assuming a tangent.
        // The calling context would be needed to determine an incoming/outgoing tangent.
    }

    result.position = targetPointPos; // Default to center snap

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