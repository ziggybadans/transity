#pragma once

#include <entt/entt.hpp>

struct AddStationToLineEvent {
    entt::entity stationEntity;
};

struct FinalizeLineEvent {};

struct CancelLineCreationEvent {};