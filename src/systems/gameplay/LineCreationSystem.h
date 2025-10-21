#pragma once

#include "ecs/ISystem.h"
#include "event/InputEvents.h"
#include "event/LineEvents.h"
#include "app/GameState.h"
#include "event/EventBus.h"
#include "components/GameLogicComponents.h"
#include "components/LineComponents.h"
#include <SFML/Graphics/Color.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <functional>

class EntityFactory;
class ColorManager;
class WorldGenerationSystem;

struct ActiveLine {
    std::vector<LinePoint> points;
};

struct LinePreview {
    std::optional<sf::Vector2f> snapPosition;
    std::optional<SnapInfo> snapInfo;
    float snapSide = 0.f; // Will be -1.f or 1.f
    std::optional<sf::Vector2f> snapTangent;
    std::vector<sf::Vector2f> curvePoints;
    std::vector<bool> validSegments;
    std::optional<float> currentSegmentGrade;
    bool currentSegmentExceedsGrade = false;
};

class LineCreationSystem : public ISystem, public IUpdatable {
public:
    explicit LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager, GameState& gameState, EventBus& eventBus, WorldGenerationSystem& worldGenerationSystem);
    ~LineCreationSystem();

    void update(sf::Time dt) override;

    void clearCurrentLine() noexcept;

private:
    void onFinalizeLine(const FinalizeLineEvent &event);
    void onMouseButtonPressed(const MouseButtonPressedEvent &event);
    void onCancelLineCreation(const CancelLineCreationEvent &event);
    void onMouseMoved(const MouseMovedEvent &event);

    void addPointToLine(const sf::Vector2f& position, entt::entity stationEntity = entt::null, std::optional<SnapInfo> snapInfo = std::nullopt, float snapSide = 0.f);
    void addPointToLine(entt::entity stationEntity);
    void finalizeLine();
    struct SegmentValidationResult {
        bool isValid = true;
        bool crossesWater = false;
        bool exceedsGrade = false;
        float maxGrade = 0.0f;
    };
    SegmentValidationResult validateSegment(const sf::Vector2f &from, const sf::Vector2f &to) const;

    entt::registry &_registry;
    EntityFactory &_entityFactory;
    ColorManager &_colorManager;
    GameState &_gameState;
    EventBus& _eventBus;
    WorldGenerationSystem& _worldGenerationSystem;
    sf::Vector2f _currentMouseWorldPos;

    entt::scoped_connection m_finalizeLineConnection;
    entt::scoped_connection m_mousePressConnection;
    entt::scoped_connection m_cancelLineCreationConnection;
    entt::scoped_connection m_mouseMoveConnection;

    static constexpr float MAX_ALLOWED_GRADE = 0.05f;
    static constexpr int SEGMENT_INTERIOR_SAMPLES = 4;
};
