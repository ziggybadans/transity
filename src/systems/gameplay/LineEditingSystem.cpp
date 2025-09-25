#include "LineEditingSystem.h"
#include "components/GameLogicComponents.h"
#include "components/WorldComponents.h"
#include "Logger.h"
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

    for (size_t i = 0; i < line.points.size(); ++i) {
        sf::Vector2f diff = line.points[i].position - event.worldPosition;
        float distanceSq = diff.x * diff.x + diff.y * diff.y;
        if (distanceSq <= 128.f) { // 8.f radius squared
            editingState.draggedPointIndex = i;
            editingState.selectedPointIndex = i;
            LOG_DEBUG("LineEditingSystem", "Dragging point %zu", i);
            return;
        }
    }

    for (size_t i = 0; i < line.points.size() - 1; ++i) {
        const auto& p1 = line.points[i].position;
        const auto& p2 = line.points[i+1].position;
        if (distanceToSegmentSq(event.worldPosition, p1, p2) < 64.f) { // 8.f radius squared
            line.points.insert(line.points.begin() + i + 1, {LinePointType::CONTROL_POINT, event.worldPosition, entt::null});
            LOG_DEBUG("LineEditingSystem", "Added point to line");
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
        auto& point = line.points[editingState.draggedPointIndex.value()];

        auto cityView = _registry.view<const PositionComponent, const CityComponent>();
        bool snapped = false;
        for (auto entity : cityView) {
            const auto& cityPosition = cityView.get<const PositionComponent>(entity);
            sf::Vector2f diff = point.position - cityPosition.coordinates;
            float distanceSq = diff.x * diff.x + diff.y * diff.y;
            if (distanceSq <= 800.f) { // 20.f radius squared
                point.position = cityPosition.coordinates;
                point.type = LinePointType::STOP;
                point.stationEntity = entity;
                snapped = true;
                break;
            }
        }

        if (!snapped) {
            point.type = LinePointType::CONTROL_POINT;
            point.stationEntity = entt::null;
        }
    }

    editingState.draggedPointIndex = std::nullopt;
    LOG_DEBUG("LineEditingSystem", "Stopped dragging point");
}

void LineEditingSystem::onMouseMoved(const MouseMovedEvent& event) {
    if (_gameState.currentInteractionMode != InteractionMode::EDIT_LINE) {
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
    if (!editingState.draggedPointIndex.has_value()) {
        return;
    }

    auto& line = _registry.get<LineComponent>(selectedLine);
    line.points[editingState.draggedPointIndex.value()].position = event.worldPosition;
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
        editingState.selectedPointIndex = std::nullopt;
        editingState.draggedPointIndex = std::nullopt;
        LOG_DEBUG("LineEditingSystem", "Deleted point from line");
    }
}