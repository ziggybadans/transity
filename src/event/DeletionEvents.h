#pragma once

#include <entt/entt.hpp>

struct DeleteEntityEvent {
    entt::entity entity;
};

struct DeleteAllEntitiesEvent {};