#include "LineDataSystem.h"
#include "components/GameLogicComponents.h"
#include "core/ServiceLocator.h"
#include "Logger.h"
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <map>
#include <cmath>

// A custom comparator for sf::Vector2f so it can be used as a map key.
struct Vector2fComparator {
    bool operator()(const sf::Vector2f& a, const sf::Vector2f& b) const {
        if (a.x < b.x) return true;
        if (a.x > b.x) return false;
        return a.y < b.y;
    }
};

LineDataSystem::LineDataSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry) {
    LOG_DEBUG("LineDataSystem", "LineDataSystem created.");
}

void LineDataSystem::update(sf::Time dt) {
    processParallelSegments();
}

void LineDataSystem::processParallelSegments() {
    // A map from a pair of station entities (representing a segment) to a list of
    // (line entity, segment index) that share this segment.
    // The station entities in the pair are always ordered to treat (A,B) and (B,A) as the same segment.
    std::map<std::pair<entt::entity, entt::entity>, std::vector<std::pair<entt::entity, size_t>>> segmentMap;

    auto lineView = _registry.view<LineComponent>();
    for (auto entity : lineView) {
        auto &line = lineView.get<LineComponent>(entity);
        if (line.stops.size() < 2) {
            line.pathOffsets.clear();
            continue;
        }

        // Ensure pathOffsets is the correct size, initialized to zero.
        line.pathOffsets.assign(line.stops.size() - 1, {0.f, 0.f});

        for (size_t i = 0; i < line.stops.size() - 1; ++i) {
            entt::entity station1 = line.stops[i];
            entt::entity station2 = line.stops[i + 1];

            // Canonical representation of the segment
            if (station1 > station2) {
                std::swap(station1, station2);
            }
            segmentMap[{station1, station2}].push_back({entity, i});
        }
    }

    const float offsetStep = 12.0f; // The distance between parallel lines

    // Now, iterate through the map and find segments shared by multiple lines
    for (const auto& [segment, lines] : segmentMap) {
        if (lines.size() > 1) {
            // This segment is shared by multiple lines.
            float totalWidth = (lines.size() - 1) * offsetStep;
            float currentOffset = -totalWidth / 2.0f;

            for (size_t i = 0; i < lines.size(); ++i) {
                entt::entity lineEntity = lines[i].first;
                size_t segmentIndex = lines[i].second;

                auto &line = _registry.get<LineComponent>(lineEntity);

                // Get the direction of the segment to calculate the perpendicular
                const auto& pos1 = _registry.get<PositionComponent>(line.stops[segmentIndex]).coordinates;
                const auto& pos2 = _registry.get<PositionComponent>(line.stops[segmentIndex + 1]).coordinates;

                sf::Vector2f direction = pos2 - pos1;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) {
                    sf::Vector2f unitDirection = direction / length;
                    sf::Vector2f perpendicular(-unitDirection.y, unitDirection.x);
                    line.pathOffsets[segmentIndex] = perpendicular * currentOffset;
                }

                currentOffset += offsetStep;
            }
        }
    }
}