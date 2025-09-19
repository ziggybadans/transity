#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <SFML/System/Vector2.hpp>

// A simple struct to represent a node in our pathfinding graph.
struct PathNode {
    entt::entity station;
    entt::entity line; // The line taken to reach this station
};

class Pathfinder {
public:
    explicit Pathfinder(entt::registry& registry);

    // Finds the shortest path between two stations.
    // Returns a vector of station entities representing the path.
    // The path includes the start and end stations.
    std::vector<entt::entity> findPath(entt::entity startStation, entt::entity endStation);

private:
    entt::registry& _registry;

    // Helper function to calculate the distance between two stations.
    float calculateDistance(entt::entity stationA, entt::entity stationB);
};