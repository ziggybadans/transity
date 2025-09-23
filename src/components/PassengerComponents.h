#pragma once

#include <entt/entt.hpp>
#include <optional>
#include <vector>

// Enum for the state of a passenger
enum class PassengerState { WAITING_FOR_TRAIN, ON_TRAIN, ARRIVED };

// A component for passenger entities.

struct PassengerComponent {
    entt::entity originStation;
    entt::entity destinationStation;
    PassengerState state = PassengerState::WAITING_FOR_TRAIN;

    // This now holds the entity the passenger is "in" (a train or a station)
    entt::entity currentContainer;
};

// A component for pathfinding information for a passenger.
struct PathComponent {
    std::vector<entt::entity> nodes;  // Sequence of stations/lines
    int currentNodeIndex = 0;
};