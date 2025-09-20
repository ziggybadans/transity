#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <optional>

// Enum for the state of a passenger
enum class PassengerState {
    WAITING_FOR_TRAIN,
    ON_TRAIN,
    ARRIVED
};

// A component for passenger entities.
struct PassengerComponent {
    entt::entity originStation;
    entt::entity destinationStation;
    PassengerState state = PassengerState::WAITING_FOR_TRAIN;
    std::optional<entt::entity> currentTrain;
};

// A component for pathfinding information for a passenger.
struct PathComponent {
    std::vector<entt::entity> nodes; // Sequence of stations/lines
    int currentNodeIndex = 0;
};