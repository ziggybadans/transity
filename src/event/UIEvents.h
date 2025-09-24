#pragma once

#include <entt/entt.hpp>

enum class Theme { Light, Dark };

struct ThemeChangedEvent {
    Theme theme;
};

struct EntitySelectedEvent {
    entt::entity entity;
};

struct EntityDeselectedEvent {};