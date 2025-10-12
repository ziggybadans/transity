#pragma once

#include "Constants.h"
#include "StrongTypes.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <optional>
#include <vector>

struct Vector2fComparatorForMap {
    bool operator()(const sf::Vector2f &a, const sf::Vector2f &b) const {
        if (a.x < b.x) return true;
        if (a.x > b.x) return false;
        return a.y < b.y;
    }
};

struct StopInfo {
    entt::entity stationEntity;
    float distanceAlongCurve;
};

struct SnapInfo {
    entt::entity snappedToEntity;
    size_t snappedToPointIndex;
};

// Represents a segment between two control points that is shared by multiple lines.
struct SharedSegment {
    // The entities of the lines that share this segment.
    std::vector<entt::entity> lines;
};

// A component for line entities.
enum class LinePointType { STOP, CONTROL_POINT };

struct LinePoint {
    LinePointType type;
    sf::Vector2f position;
    entt::entity stationEntity = entt::null;
    std::optional<SnapInfo> snapInfo;
    float snapSide = 0.f;  // Add this. 0 for center, -1 for left, 1 for right.
};

struct LineComponent {
    sf::Color color;
    std::vector<LinePoint> points;
    std::vector<sf::Vector2f> pathOffsets;
    std::vector<sf::Vector2f> curvePoints;
    std::vector<size_t> curveSegmentIndices;
    std::vector<StopInfo> stops;  // Add this line
    float totalDistance = 0.0f;
    Thickness thickness = {Constants::DEFAULT_LINE_THICKNESS};

    // A map where the key is the segment index and the value is a pointer
    // to the corresponding SharedSegment in the global context.
    std::map<size_t, SharedSegment *> sharedSegments;
};

// A component to manage the state of line editing.
struct LineEditingComponent {
    std::optional<size_t> selectedPointIndex;
    std::optional<size_t> draggedPointIndex;
    std::optional<sf::Vector2f> originalPointPosition;
    std::optional<sf::Vector2f> snapPosition;
    std::optional<SnapInfo> snapInfo;
    float snapSide = 0.f;
    std::optional<sf::Vector2f> snapTangent;
};

// A global context structure to hold all shared segments in the game world.
struct SharedSegmentsContext {
    std::map<std::pair<sf::Vector2f, sf::Vector2f>, SharedSegment,
             std::function<bool(const std::pair<sf::Vector2f, sf::Vector2f> &,
                                const std::pair<sf::Vector2f, sf::Vector2f> &)>>
        segments{[](const std::pair<sf::Vector2f, sf::Vector2f> &a,
                    const std::pair<sf::Vector2f, sf::Vector2f> &b) {
            Vector2fComparatorForMap cmp;
            if (cmp(a.first, b.first)) return true;
            if (cmp(b.first, a.first)) return false;
            return cmp(a.second, b.second);
        }};
};

// A tag for stations that are part of a line currently being created.
struct ActiveLineStationTag {
    StationOrder order = {0};
};