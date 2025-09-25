#include "LineCreationSystem.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "ecs/EntityFactory.h"
#include "render/ColorManager.h"
#include "imgui.h"
#include <algorithm>
#include <utility>

LineCreationSystem::LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager, GameState& gameState, EventBus& eventBus)
    : _registry(registry), _entityFactory(entityFactory),
      _colorManager(colorManager), _gameState(gameState), _eventBus(eventBus) {

    m_finalizeLineConnection = _eventBus.sink<FinalizeLineEvent>()
                                   .connect<&LineCreationSystem::onFinalizeLine>(this);
    m_mousePressConnection = _eventBus.sink<MouseButtonPressedEvent>()
                                   .connect<&LineCreationSystem::onMouseButtonPressed>(this);
    m_cancelLineCreationConnection = _eventBus.sink<CancelLineCreationEvent>()
                                   .connect<&LineCreationSystem::onCancelLineCreation>(this);
  LOG_DEBUG("LineCreationSystem", "LineCreationSystem created and connected to EventBus.");
}

LineCreationSystem::~LineCreationSystem() {
    m_finalizeLineConnection.release();
    m_mousePressConnection.release();
    m_cancelLineCreationConnection.release();
    LOG_DEBUG("LineCreationSystem", "LineCreationSystem destroyed and disconnected from EventBus.");
}

void LineCreationSystem::onMouseButtonPressed(const MouseButtonPressedEvent &event) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (_gameState.currentInteractionMode == InteractionMode::CREATE_LINE
        && event.button == sf::Mouse::Button::Left) {
        LOG_DEBUG("LineCreationSystem", "Mouse click in CREATE_LINE mode at world (%.1f, %.1f).",
                  event.worldPosition.x, event.worldPosition.y);

        auto view = _registry.view<PositionComponent, ClickableComponent, CityComponent>();
        for (auto entity_id : view) {
            const auto &pos = view.get<PositionComponent>(entity_id);
            const auto &clickable = view.get<ClickableComponent>(entity_id);

            sf::Vector2f diff = event.worldPosition - pos.coordinates;
            float distanceSquared = (diff.x * diff.x) + (diff.y * diff.y);

            if (distanceSquared
                <= clickable.boundingRadius.value * clickable.boundingRadius.value) {
                LOG_DEBUG("LineCreationSystem", "Station entity %u clicked.",
                          static_cast<unsigned int>(entity_id));

                addPointToLine(pos.coordinates, entity_id);
                return;
            }
        }

        addPointToLine(event.worldPosition);
    }
}

void LineCreationSystem::addPointToLine(const sf::Vector2f& position, entt::entity stationEntity) {
    auto& activeLine = _registry.ctx().get<ActiveLine>();

    if (activeLine.points.empty() && stationEntity == entt::null) {
        LOG_WARN("LineCreationSystem", "The first point of a line must be a station.");
        return;
    }

    if (stationEntity != entt::null) {
        if (!activeLine.points.empty()) {
            const auto& lastPoint = activeLine.points.back();
            if (lastPoint.type == LinePointType::STOP && lastPoint.stationEntity == stationEntity) {
                LOG_WARN("LineCreationSystem",
                         "Station %u is already the last point in the active line.",
                         static_cast<unsigned int>(stationEntity));
                return;
            }
        }
        activeLine.points.push_back({LinePointType::STOP, position, stationEntity});
        LOG_DEBUG("LineCreationSystem", "Station %u added to active line.", static_cast<unsigned int>(stationEntity));
    } else {
        activeLine.points.push_back({LinePointType::CONTROL_POINT, position});
        LOG_DEBUG("LineCreationSystem", "Control point added to active line at (%.1f, %.1f).", position.x, position.y);
    }
}

void LineCreationSystem::finalizeLine() {
    auto& activeLine = _registry.ctx().get<ActiveLine>();

    if (activeLine.points.size() < 2) {
        LOG_WARN("LineCreationSystem",
                 "Not enough points to finalize line. Need at least 2, have %zu.",
                 activeLine.points.size());
        clearCurrentLine();
        return;
    }

    if (activeLine.points.back().type != LinePointType::STOP) {
        LOG_WARN("LineCreationSystem", "Cannot finalize line: the last point must be a station.");
        return;
    }

    LOG_DEBUG("LineCreationSystem", "Finalizing line with %zu points.",
              activeLine.points.size());

    sf::Color chosenColor = _colorManager.getNextLineColor();
    entt::entity lineEntity = _entityFactory.createLine(activeLine.points, chosenColor);

    if (lineEntity == entt::null) {
        LOG_ERROR("LineCreationSystem", "Failed to create line entity.");
        clearCurrentLine();
        return;
    }

    for (const auto& point : activeLine.points) {
        if (point.type == LinePointType::STOP) {
            auto& stationComp = _registry.get<CityComponent>(point.stationEntity);
            stationComp.connectedLines.push_back(lineEntity);
            LOG_DEBUG("LineCreationSystem", "Connected line %u to station %u",
                      static_cast<unsigned int>(lineEntity), static_cast<unsigned int>(point.stationEntity));
        }
    }

    LOG_DEBUG("LineCreationSystem", "Created line entity with ID: %u.",
             static_cast<unsigned int>(lineEntity));

    clearCurrentLine();
}

void LineCreationSystem::clearCurrentLine() noexcept {
    LOG_DEBUG("LineCreationSystem", "Clearing active line.");
    _registry.ctx().erase<ActiveLine>();
    _registry.ctx().emplace<ActiveLine>();
}

void LineCreationSystem::onFinalizeLine(const FinalizeLineEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing FinalizeLineEvent.");
    finalizeLine();
}

void LineCreationSystem::update(sf::Time dt) {
    if (_gameState.currentInteractionMode == InteractionMode::CREATE_LINE && !_registry.ctx().contains<ActiveLine>()) {
        _registry.ctx().emplace<ActiveLine>();
    }
}

void LineCreationSystem::onCancelLineCreation(const CancelLineCreationEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing CancelLineCreationEvent.");
    clearCurrentLine();
}