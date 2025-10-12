#include "core/Pathfinder.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "components/LineComponents.h"
#include <cmath>
#include <limits>
#include <map>
#include <queue>
#include <vector>

Pathfinder::Pathfinder(entt::registry &registry) : _registry(registry) {}

float Pathfinder::calculateDistance(const sf::Vector2f &posA, const sf::Vector2f &posB) {
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
            break;
        }

        if (!_registry.valid(u) || !_registry.all_of<CityComponent>(u)) continue;
        auto &city = _registry.get<CityComponent>(u);

        for (auto lineEntity : city.connectedLines) {
            if (!_registry.valid(lineEntity) || !_registry.all_of<LineComponent>(lineEntity))
                continue;
            auto &line = _registry.get<LineComponent>(lineEntity);

            auto it = std::find_if(line.points.begin(), line.points.end(), [u](const LinePoint &p) {
                return p.type == LinePointType::STOP && p.stationEntity == u;
            });
            if (it == line.points.end()) continue;

            size_t currentIndex = std::distance(line.points.begin(), it);

            // Check previous stops on the line
            float distanceToPrev = 0;
            for (int i = currentIndex - 1; i >= 0; --i) {
                distanceToPrev +=
                    calculateDistance(line.points[i + 1].position, line.points[i].position);
                if (line.points[i].type == LinePointType::STOP) {
                    entt::entity v = line.points[i].stationEntity;
                    if (distances[u] + distanceToPrev < distances[v]) {
                        distances[v] = distances[u] + distanceToPrev;
                        predecessors[v] = u;
                        pq.push({distances[v], v});
                    }
                    break;
                }
            }

            // Check next stops on the line
            float distanceToNext = 0;
            for (size_t i = currentIndex + 1; i < line.points.size(); ++i) {
                distanceToNext +=
                    calculateDistance(line.points[i - 1].position, line.points[i].position);
                if (line.points[i].type == LinePointType::STOP) {
                    entt::entity v = line.points[i].stationEntity;
                    if (distances[u] + distanceToNext < distances[v]) {
                        distances[v] = distances[u] + distanceToNext;
                        predecessors[v] = u;
                        pq.push({distances[v], v});
                    }
                    break;
                }
            }
        }
    }

    std::vector<entt::entity> path;
    entt::entity current = endStation;

    // If start and end are the same, path is empty
    if (startStation == endStation) {
        return path;
    }

    while (predecessors.count(current)) {
        path.insert(path.begin(), current);
        current = predecessors[current];
        if (current == startStation) {
            break;  // We've reached the start, don't add it to the path
        }
    }

    if (path.empty() || (current != startStation && !predecessors.count(current))) {
        LOG_WARN("Pathfinder", "No path found from station %u to %u.",
                 static_cast<unsigned>(startStation), static_cast<unsigned>(endStation));
        return {};
    }

    LOG_DEBUG("Pathfinder", "Path found with %zu stops.", path.size());
    return path;
}