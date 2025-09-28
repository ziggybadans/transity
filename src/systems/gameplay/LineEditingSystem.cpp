#include "LineEditingSystem.h"
#include "components/GameLogicComponents.h"
#include "components/WorldComponents.h"
#include "Logger.h"
#include "Constants.h"
#include "core/Curve.h"
#include "event/LineEvents.h"
#include <algorithm>
#include <limits>

namespace {
    float distanceToSegmentSq(sf::Vector2f p, sf::Vector2f v, sf::Vector2f w) {
        float l2 = (w.x - v.x) * (w.x - v.x) + (w.y - v.y) * (w.y - v.y);
        if (l2 == 0.0) {
            return (p.x - v.x) * (p.x - v.x) + (p.y - v.y) * (p.y - v.y);
        }
        float t = std::max(0.f, std::min(1.f, ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2));
        sf::Vector2f projection = v + t * (w - v);
        sf::Vector2f d = p - projection;
        return d.x * d.x + d.y * d.y;
    }

    void regenerateCurve(LineComponent& line) {
        if (line.points.size() < 2) {
            line.curvePoints.clear();
            line.curveSegmentIndices.clear();
            return;
        }

        std::vector<sf::Vector2f> controlPoints;
        controlPoints.reserve(line.points.size());
        for (const auto& p : line.points) {
            controlPoints.push_back(p.position);
        }
        
        CurveData curveData = Curve::generateMetroCurve(controlPoints, Constants::METRO_CURVE_RADIUS);
        line.curvePoints = curveData.points;
        line.totalDistance = Curve::calculateCurveLength(line.curvePoints);
        line.stops = Curve::calculateStopInfo(line.points, line.curvePoints); // Add this
    }
}

LineEditingSystem::LineEditingSystem(entt::registry& registry, EventBus& eventBus, GameState& gameState)
    : _registry(registry), _eventBus(eventBus), _gameState(gameState) {
    _interactionModeChangeConnection = _eventBus.sink<InteractionModeChangeEvent>().connect<&LineEditingSystem::onInteractionModeChange>(this);
    _mouseButtonPressedConnection = _eventBus.sink<MouseButtonPressedEvent>().connect<&LineEditingSystem::onMouseButtonPressed>(this);
    _mouseButtonReleasedConnection = _eventBus.sink<MouseButtonReleasedEvent>().connect<&LineEditingSystem::onMouseButtonReleased>(this);
    _mouseMovedConnection = _eventBus.sink<MouseMovedEvent>().connect<&LineEditingSystem::onMouseMoved>(this);
    _keyPressedConnection = _eventBus.sink<KeyPressedEvent>().connect<&LineEditingSystem::onKeyPressed>(this);
}

LineEditingSystem::~LineEditingSystem() {
    _eventBus.sink<InteractionModeChangeEvent>().disconnect(this);
    _eventBus.sink<MouseButtonPressedEvent>().disconnect(this);
    _eventBus.sink<MouseButtonReleasedEvent>().disconnect(this);
    _eventBus.sink<MouseMovedEvent>().disconnect(this);
    _eventBus.sink<KeyPressedEvent>().disconnect(this);
}

void LineEditingSystem::onInteractionModeChange(const InteractionModeChangeEvent& event) {
    if (event.newMode == InteractionMode::EDIT_LINE) {
        if (_gameState.selectedEntity.has_value()) {
            if (_registry.all_of<LineComponent>(_gameState.selectedEntity.value())) {
                 _registry.emplace_or_replace<LineEditingComponent>(_gameState.selectedEntity.value());
            }
        }
    } else {
        auto view = _registry.view<LineEditingComponent>();
        for (auto entity : view) {
            _registry.remove<LineEditingComponent>(entity);
        }
    }
}

void LineEditingSystem::onMouseButtonPressed(const MouseButtonPressedEvent& event) {
    if (_gameState.currentInteractionMode != InteractionMode::EDIT_LINE || event.button != sf::Mouse::Button::Left) {
        return;
    }

    if (!_gameState.selectedEntity.has_value()) {
        return;
    }

    entt::entity selectedLine = _gameState.selectedEntity.value();
    if (!_registry.all_of<LineComponent, LineEditingComponent>(selectedLine)) {
        return;
    }

    auto& line = _registry.get<LineComponent>(selectedLine);
    auto& editingState = _registry.get<LineEditingComponent>(selectedLine);
    editingState.selectedPointIndex = std::nullopt;
    editingState.originalPointPosition = std::nullopt;

    for (size_t i = 0; i < line.points.size(); ++i) {
        sf::Vector2f diff = line.points[i].position - event.worldPosition;
        float distanceSq = diff.x * diff.x + diff.y * diff.y;
        if (distanceSq <= 128.f) { // 8.f radius squared
            editingState.draggedPointIndex = i;
            editingState.selectedPointIndex = i;
            
            bool isEndpoint = (i == 0 || i == line.points.size() - 1);
            if (isEndpoint && line.points[i].type == LinePointType::STOP) {
                editingState.originalPointPosition = line.points[i].position;
            }

            LOG_DEBUG("LineEditingSystem", "Dragging point %zu", i);
            return;
        }
    }

    for (size_t i = 0; i < line.points.size() - 1; ++i) {
        const auto& p1 = line.points[i].position;
        const auto& p2 = line.points[i+1].position;
        if (distanceToSegmentSq(event.worldPosition, p1, p2) < 64.f) { // 8.f radius squared
            line.points.insert(line.points.begin() + i + 1, {LinePointType::CONTROL_POINT, event.worldPosition, entt::null});
            regenerateCurve(line);
            _eventBus.enqueue<LineModifiedEvent>({selectedLine});
            LOG_DEBUG("LineEditingSystem", "Added point to line and regenerated curve");
            return;
        }
    }
}

void LineEditingSystem::onMouseButtonReleased(const MouseButtonReleasedEvent& event) {
    if (_gameState.currentInteractionMode != InteractionMode::EDIT_LINE || event.button != sf::Mouse::Button::Left) {
        return;
    }

    if (!_gameState.selectedEntity.has_value()) {
        return;
    }

    entt::entity selectedLine = _gameState.selectedEntity.value();
    if (!_registry.all_of<LineComponent, LineEditingComponent>(selectedLine)) {
        return;
    }

    auto& editingState = _registry.get<LineEditingComponent>(selectedLine);

    if (editingState.draggedPointIndex.has_value()) {
        auto& line = _registry.get<LineComponent>(selectedLine);
        size_t draggedIndex = editingState.draggedPointIndex.value();
        auto& point = line.points[draggedIndex];

        if (editingState.snapInfo && editingState.snapPosition) {
            point.position = editingState.snapPosition.value();
            point.snapInfo = editingState.snapInfo;
            point.snapSide = editingState.snapSide;

            if (editingState.snapInfo->snappedToPointIndex == (size_t)-1) { // Snapped to a city
                point.type = LinePointType::STOP;
                point.stationEntity = editingState.snapInfo->snappedToEntity;
            } else { // Snapped to a control point
                point.type = LinePointType::CONTROL_POINT;
                point.stationEntity = entt::null;
            }
        } else {
            if (editingState.originalPointPosition.has_value()) {
                point.position = editingState.originalPointPosition.value();
            } else {
                point.type = LinePointType::CONTROL_POINT;
                point.stationEntity = entt::null;
                point.snapInfo.reset();
                point.snapSide = 0.f;
            }
        }

        regenerateCurve(line);
        _eventBus.enqueue<LineModifiedEvent>({selectedLine});
    }

    editingState.draggedPointIndex = std::nullopt;
    editingState.originalPointPosition = std::nullopt;
    editingState.snapPosition.reset();
    editingState.snapInfo.reset();
    editingState.snapSide = 0.f;
    editingState.snapTangent.reset();
    LOG_DEBUG("LineEditingSystem", "Stopped dragging point");
}

void LineEditingSystem::onMouseMoved(const MouseMovedEvent& event) {
    if (_gameState.currentInteractionMode != InteractionMode::EDIT_LINE) {
        return;
    }

    if (!_gameState.selectedEntity.has_value()) {
        return;
    }

    entt::entity selectedLineEntity = _gameState.selectedEntity.value();
    if (!_registry.all_of<LineComponent, LineEditingComponent>(selectedLineEntity)) {
        return;
    }

    auto& editingState = _registry.get<LineEditingComponent>(selectedLineEntity);
    if (!editingState.draggedPointIndex.has_value()) {
        return;
    }

    editingState.snapPosition.reset();
    editingState.snapInfo.reset();
    editingState.snapSide = 0.f;
    editingState.snapTangent.reset();

    sf::Vector2f mousePos = event.worldPosition;

    const float SNAP_RADIUS_SQUARED = Constants::LINE_SNAP_RADIUS * Constants::LINE_SNAP_RADIUS;
    float closestDistSq = SNAP_RADIUS_SQUARED;

    auto lineView = _registry.view<const LineComponent>();
    for (auto entity : lineView) {
        const auto& lineComp = lineView.get<const LineComponent>(entity);
        for (size_t i = 0; i < lineComp.points.size(); ++i) {
            if (entity == selectedLineEntity && i == editingState.draggedPointIndex.value()) {
                continue;
            }

            const auto& point = lineComp.points[i];
            if (point.type == LinePointType::CONTROL_POINT) {
                sf::Vector2f diff = mousePos - point.position;
                float distSq = diff.x * diff.x + diff.y * diff.y;
                if (distSq < closestDistSq) {
                    closestDistSq = distSq;
                    editingState.snapInfo = SnapInfo{entity, i};
                }
            }
        }
    }

    auto cityView = _registry.view<const CityComponent, const PositionComponent>();
    for (auto entity : cityView) {
        const auto& pos = cityView.get<const PositionComponent>(entity);
        sf::Vector2f diff = mousePos - pos.coordinates;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            editingState.snapInfo = SnapInfo{entity, (size_t)-1};
        }
    }

    sf::Vector2f finalPos = mousePos;

    if (editingState.snapInfo) {
        sf::Vector2f targetPointPos;
        sf::Vector2f tangent;
        bool tangentFound = false;

        if (editingState.snapInfo->snappedToPointIndex != (size_t)-1) {
            const auto& targetLine = _registry.get<LineComponent>(editingState.snapInfo->snappedToEntity);
            targetPointPos = targetLine.points[editingState.snapInfo->snappedToPointIndex].position;

            sf::Vector2f p_prev, p_next;
            if (editingState.snapInfo->snappedToPointIndex > 0) {
                p_prev = targetLine.points[editingState.snapInfo->snappedToPointIndex - 1].position;
            } else if (targetLine.points.size() > 1) {
                p_prev = targetPointPos - (targetLine.points[1].position - targetPointPos);
            }

            if (editingState.snapInfo->snappedToPointIndex < targetLine.points.size() - 1) {
                p_next = targetLine.points[editingState.snapInfo->snappedToPointIndex + 1].position;
            } else if (targetLine.points.size() > 1) {
                p_next = targetPointPos + (targetPointPos - targetLine.points[targetLine.points.size() - 2].position);
            }
            tangent = p_next - p_prev;
            tangentFound = true;
        } else {
            entt::entity cityEntity = editingState.snapInfo->snappedToEntity;
            targetPointPos = _registry.get<PositionComponent>(cityEntity).coordinates;
            
            auto& editedLine = _registry.get<LineComponent>(selectedLineEntity);
            size_t draggedIdx = editingState.draggedPointIndex.value();
            if (!tangentFound) {
                if (draggedIdx > 0) {
                    tangent = targetPointPos - editedLine.points[draggedIdx - 1].position;
                } else if (editedLine.points.size() > 1) {
                    tangent = editedLine.points[draggedIdx + 1].position - targetPointPos;
                }
                tangentFound = true;
            }
        }

        if (tangentFound) {
            if (auto len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y); len > 0) {
                tangent /= len;
            }
            editingState.snapTangent = tangent;

            sf::Vector2f perpendicular = {-tangent.y, tangent.x};
            sf::Vector2f mouseVec = mousePos - targetPointPos;
            float perp_dist = mouseVec.x * perpendicular.x + mouseVec.y * perpendicular.y;

            if (std::abs(perp_dist) < Constants::LINE_CENTER_SNAP_RADIUS) {
                editingState.snapSide = 0.f;
                editingState.snapPosition = targetPointPos;
            } else {
                editingState.snapSide = (perp_dist > 0) ? 1.f : -1.f;
                sf::Vector2f sideOffset = perpendicular * editingState.snapSide * Constants::LINE_PARALLEL_OFFSET;
                editingState.snapPosition = targetPointPos + sideOffset;
            }
            finalPos = editingState.snapPosition.value();
        } else {
             editingState.snapPosition = targetPointPos;
             finalPos = targetPointPos;
        }
    }

    auto& line = _registry.get<LineComponent>(selectedLineEntity);
    line.points[editingState.draggedPointIndex.value()].position = finalPos;

    regenerateCurve(line);
}

void LineEditingSystem::onKeyPressed(const KeyPressedEvent& event) {
    if (_gameState.currentInteractionMode != InteractionMode::EDIT_LINE || (event.code != sf::Keyboard::Key::Delete && event.code != sf::Keyboard::Key::Backspace)) {
        return;
    }

    if (!_gameState.selectedEntity.has_value()) {
        return;
    }

    entt::entity selectedLine = _gameState.selectedEntity.value();
    if (!_registry.all_of<LineComponent, LineEditingComponent>(selectedLine)) {
        return;
    }

    auto& editingState = _registry.get<LineEditingComponent>(selectedLine);
    if (!editingState.selectedPointIndex.has_value()) {
        return;
    }

    auto& line = _registry.get<LineComponent>(selectedLine);
    if (line.points.size() > 2) {
        line.points.erase(line.points.begin() + editingState.selectedPointIndex.value());
        regenerateCurve(line);
        _eventBus.enqueue<LineModifiedEvent>({selectedLine});
        editingState.selectedPointIndex = std::nullopt;
        editingState.draggedPointIndex = std::nullopt;
        LOG_DEBUG("LineEditingSystem", "Deleted point from line");
    }
}