#pragma once

#include "entt/entt.hpp"

struct AddStationToLineEvent {
    entt::entity stationEntity;
};

struct FinalizeLineEvent {};

struct CancelLineCreationEvent {};

struct AddTrainToLineEvent {
    entt::entity lineEntity;
};

struct LineModifiedEvent {
    entt::entity lineEntity;
};