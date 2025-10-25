#pragma once

#include <entt/entt.hpp>
#include <string>

enum class Theme { Light, Dark };

struct ThemeChangedEvent {
    Theme theme;
};

struct EntitySelectedEvent {
    entt::entity entity;
};

struct EntityDeselectedEvent {};

struct SaveGameRequestEvent {
    std::string path;
};

struct LoadGameRequestEvent {
    std::string path;
};
