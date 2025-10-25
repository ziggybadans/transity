#pragma once

#include "components/GameLogicComponents.h"
#include "components/LineComponents.h"
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>
#include <optional>

struct SnapResult {
    sf::Vector2f position;
    sf::Vector2f tangent;
    float side; // -1 for left, 0 for center, 1 for right
    SnapInfo info;
};

class SnapHelper {
public:
    // Finds the best snap point near the mouse position.
    // Optionally provide the previous point's position to help determine tangents.
    // Optionally ignores a specific point on a specific line (e.g., the one being dragged).
    static std::optional<SnapResult> findSnap(
        entt::registry &registry, const sf::Vector2f &mousePos,
        std::optional<sf::Vector2f> previousPointPos = std::nullopt,
        std::optional<std::pair<entt::entity, size_t>> ignorePoint = std::nullopt);
};