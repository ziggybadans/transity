#include "LineCreationSystem.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "ecs/EntityFactory.h"
#include "render/ColorManager.h"
#include "imgui.h"
#include "Constants.h"
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
    m_mouseMoveConnection = _eventBus.sink<MouseMovedEvent>() // ADD THIS
                                  .connect<&LineCreationSystem::onMouseMoved>(this); // ADD THIS
    _registry.ctx().emplace<LinePreview>();
    LOG_DEBUG("LineCreationSystem", "LineCreationSystem created and connected to EventBus.");
}

LineCreationSystem::~LineCreationSystem() {
    m_finalizeLineConnection.release();
    m_mousePressConnection.release();
    m_cancelLineCreationConnection.release();
    m_mouseMoveConnection.release(); // ADD THIS
    LOG_DEBUG("LineCreationSystem", "LineCreationSystem destroyed and disconnected from EventBus.");
}

void LineCreationSystem::onMouseMoved(const MouseMovedEvent &event) {
    _currentMouseWorldPos = event.worldPosition;
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

        auto& preview = _registry.ctx().get<LinePreview>();
        if (preview.snapPosition && preview.snapInfo) {
            addPointToLine(*preview.snapPosition, entt::null, preview.snapInfo);
        } else {
            addPointToLine(event.worldPosition);
        }
    }
}


void LineCreationSystem::addPointToLine(const sf::Vector2f& position, entt::entity stationEntity, std::optional<SnapInfo> snapInfo) {
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
        activeLine.points.push_back({LinePointType::CONTROL_POINT, position, entt::null, snapInfo});
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

    auto& newLineComp = _registry.get<LineComponent>(lineEntity);
    for (size_t i = 0; i < activeLine.points.size() -1; ++i) {
        const auto& point1 = activeLine.points[i];
        const auto& point2 = activeLine.points[i+1];

        if (point1.snapInfo && point2.snapInfo && point1.snapInfo->snappedToEntity == point2.snapInfo->snappedToEntity) {
            
            const auto& snappedLineComp = _registry.get<LineComponent>(point1.snapInfo->snappedToEntity);
            
            size_t snappedIndex1 = point1.snapInfo->snappedToPointIndex;
            size_t snappedIndex2 = point2.snapInfo->snappedToPointIndex;

            if (snappedIndex1 > snappedIndex2) std::swap(snappedIndex1, snappedIndex2);

            for (size_t j = snappedIndex1; j < snappedIndex2; ++j) {
                const auto& snappedPoint = snappedLineComp.points[j];
                const auto& nextSnappedPoint = snappedLineComp.points[j+1];

                sf::Vector2f tangent = nextSnappedPoint.position - snappedPoint.position;
                float len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);
                if (len > 0) tangent /= len;
                else continue;

                sf::Vector2f perpendicular = {-tangent.y, tangent.x};

                sf::Vector2f dir_to_new_point = point1.position - snappedLineComp.points[point1.snapInfo->snappedToPointIndex].position;
                float dot_product = dir_to_new_point.x * perpendicular.x + dir_to_new_point.y * perpendicular.y;

                if (j < newLineComp.pathOffsets.size()) {
                    if (dot_product > 0) {
                        newLineComp.pathOffsets[j] = perpendicular * Constants::LINE_PARALLEL_OFFSET;
                    } else {
                        newLineComp.pathOffsets[j] = -perpendicular * Constants::LINE_PARALLEL_OFFSET;
                    }
                }
            }
        }
    }

    if (newLineComp.points.size() > 1) {
        // Check first segment
        if (newLineComp.pathOffsets.size() > 0 && newLineComp.pathOffsets[0] != sf::Vector2f(0,0)) {
            newLineComp.points[0].position += newLineComp.pathOffsets[0];
        }
        // Check last segment
        size_t lastSegmentIndex = newLineComp.curveSegmentIndices.back();
        if (newLineComp.pathOffsets.size() > lastSegmentIndex && newLineComp.pathOffsets[lastSegmentIndex] != sf::Vector2f(0,0)) {
            newLineComp.points.back().position += newLineComp.pathOffsets[lastSegmentIndex];
        }
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
    if (_gameState.currentInteractionMode != InteractionMode::CREATE_LINE) {
        return;
    }
    if (!_registry.ctx().contains<ActiveLine>()) {
        _registry.ctx().emplace<ActiveLine>();
    }

    auto& preview = _registry.ctx().get<LinePreview>();
    preview.snapPosition.reset();
    preview.snapInfo.reset();

    sf::Vector2f mousePos = _currentMouseWorldPos;

    const float SNAP_RADIUS_SQUARED = Constants::LINE_SNAP_RADIUS * Constants::LINE_SNAP_RADIUS;
    float closestDistSq = SNAP_RADIUS_SQUARED;

    auto lineView = _registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto& lineComp = lineView.get<const LineComponent>(entity);
        for (size_t i = 0; i < lineComp.points.size(); ++i) {
            const auto& point = lineComp.points[i];
            if (point.type == LinePointType::CONTROL_POINT) {
                sf::Vector2f diff = mousePos - point.position;
                float distSq = diff.x * diff.x + diff.y * diff.y;

                if (distSq < closestDistSq) {
                    closestDistSq = distSq;
                    preview.snapInfo = SnapInfo{entity, i};
                }
            }
        }
    }

    if (preview.snapInfo) {
        const auto& targetLine = _registry.get<LineComponent>(preview.snapInfo->snappedToEntity);
        const auto& targetPoint = targetLine.points[preview.snapInfo->snappedToPointIndex];

        sf::Vector2f p_prev, p_next;
        if (preview.snapInfo->snappedToPointIndex > 0) {
            p_prev = targetLine.points[preview.snapInfo->snappedToPointIndex - 1].position;
        } else if (targetLine.points.size() > 1) {
            p_prev = targetPoint.position - (targetLine.points[1].position - targetPoint.position);
        } else {
            p_prev = targetPoint.position;
        }

        if (preview.snapInfo->snappedToPointIndex < targetLine.points.size() - 1) {
            p_next = targetLine.points[preview.snapInfo->snappedToPointIndex + 1].position;
        } else if (targetLine.points.size() > 1) {
            p_next = targetPoint.position + (targetPoint.position - targetLine.points[targetLine.points.size() - 2].position);
        } else {
            p_next = targetPoint.position;
        }

        sf::Vector2f tangent = p_next - p_prev;
        float len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);
        if (len > 0) {
            tangent /= len;
        } else {
            tangent = {1.f, 0.f};
        }
        
        sf::Vector2f perpendicular = {-tangent.y, tangent.x};
        preview.snapPosition = targetPoint.position + perpendicular * Constants::LINE_PARALLEL_OFFSET;
    }
}

void LineCreationSystem::onCancelLineCreation(const CancelLineCreationEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing CancelLineCreationEvent.");
    clearCurrentLine();
}