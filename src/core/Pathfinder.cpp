#include "core/Pathfinder.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include <cmath>
#include <limits>
#include <map>
#include <queue>
#include <vector>

Pathfinder::Pathfinder(entt::registry &registry) : _registry(registry) {}

float Pathfinder::calculateDistance(entt::entity stationA, entt::entity stationB) {
    if (!_registry.valid(stationA) || !_registry.valid(stationB)
        || !_registry.all_of<PositionComponent>(stationA)
        || !_registry.all_of<PositionComponent>(stationB)) {
        return std::numeric_limits<float>::max();
    }
    const auto &posA = _registry.get<PositionComponent>(stationA).coordinates;
    const auto &posB = _registry.get<PositionComponent>(stationB).coordinates;
    sf::Vector2f diff = posB - posA;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

std::vector<entt::entity> Pathfinder::findPath(entt::entity startStation, entt::entity endStation) {
    std::map<entt::entity, float> distances;
    std::map<entt::entity, entt::entity> predecessors;
    std::priority_queue<std::pair<float, entt::entity>, std::vector<std::pair<float, entt::entity>>,
                        std::greater<std::pair<float, entt::entity>>>
        pq;

    auto stationView = _registry.view<CityComponent>();
    for (auto stationEntity : stationView) {
        distances[stationEntity] = std::numeric_limits<float>::max();
    }

    distances[startStation] = 0.0f;
    pq.push({0.0f, startStation});

    while (!pq.empty()) {
        entt::entity u = pq.top().second;
        pq.pop();

        if (u == endStation) {
            break;  // Found the shortest path
        }

        if (!_registry.valid(u) || !_registry.all_of<CityComponent>(u)) continue;
        auto &city = _registry.get<CityComponent>(u);

        // Explore neighbors (adjacent stations on the same line)
        for (auto lineEntity : city.connectedLines) {
            if (!_registry.valid(lineEntity) || !_registry.all_of<LineComponent>(lineEntity))
                continue;
            auto &line = _registry.get<LineComponent>(lineEntity);

            auto it = std::find(line.stops.begin(), line.stops.end(), u);
            if (it == line.stops.end()) continue;

            size_t currentIndex = std::distance(line.stops.begin(), it);

            // Check previous stop on the line
            if (currentIndex > 0) {
                entt::entity v = line.stops[currentIndex - 1];
                float weight = calculateDistance(u, v);
                if (distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    predecessors[v] = u;
                    pq.push({distances[v], v});
                }
            }

            // Check next stop on the line
            if (currentIndex < line.stops.size() - 1) {
                entt::entity v = line.stops[currentIndex + 1];
                float weight = calculateDistance(u, v);
                if (distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    predecessors[v] = u;
                    pq.push({distances[v], v});
                }
            }
        }
    }

    // Reconstruct path
    std::vector<entt::entity> path;
    entt::entity current = endStation;
    while (current != entt::null && predecessors.count(current)) {
        path.insert(path.begin(), current);
        current = predecessors[current];
    }
    if (current == startStation) {
        path.insert(path.begin(), startStation);
    }

    if (path.empty() || path.front() != startStation) {
        LOG_WARN("Pathfinder", "No path found from station %u to %u.",
                 static_cast<unsigned>(startStation), static_cast<unsigned>(endStation));
        return {};  // Return empty path if none found
    }

    LOG_DEBUG("Pathfinder", "Path found with %zu stops.", path.size());
    return path;
}
