#include "LineCreationSystem.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "render/ColorManager.h"
#include <algorithm>
#include <utility>

LineCreationSystem::LineCreationSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry), _entityFactory(serviceLocator.entityFactory),
      _colorManager(serviceLocator.colorManager), _gameState(serviceLocator.gameState) {

    m_finalizeLineConnection = serviceLocator.eventBus.sink<FinalizeLineEvent>()
                                   .connect<&LineCreationSystem::onFinalizeLine>(this);
    m_mousePressConnection = serviceLocator.eventBus.sink<MouseButtonPressedEvent>()
                                 .connect<&LineCreationSystem::onMouseButtonPressed>(this);
    LOG_INFO("LineCreationSystem", "LineCreationSystem created and connected to EventBus.");
}

LineCreationSystem::~LineCreationSystem() {

    m_finalizeLineConnection.release();
    m_mousePressConnection.release();
    LOG_INFO("LineCreationSystem", "LineCreationSystem destroyed and disconnected from EventBus.");
}

void LineCreationSystem::onMouseButtonPressed(const MouseButtonPressedEvent &event) {
    if (_gameState.currentInteractionMode == InteractionMode::CREATE_LINE
        && event.button == sf::Mouse::Button::Left) {
        LOG_DEBUG("LineCreationSystem", "Mouse click in CREATE_LINE mode at world (%.1f, %.1f).",
                  event.worldPosition.x, event.worldPosition.y);

        auto view = _registry.view<PositionComponent, ClickableComponent>();
        for (auto entity_id : view) {
            const auto &pos = view.get<PositionComponent>(entity_id);
            const auto &clickable = view.get<ClickableComponent>(entity_id);

            sf::Vector2f diff = event.worldPosition - pos.coordinates;
            float distanceSquared = (diff.x * diff.x) + (diff.y * diff.y);

            if (distanceSquared
                <= clickable.boundingRadius.value * clickable.boundingRadius.value) {
                LOG_DEBUG("LineCreationSystem", "Station entity %u clicked.",
                          static_cast<unsigned int>(entity_id));

                addStationToLine(entity_id);
                return;
            }
        }
        LOG_TRACE("LineCreationSystem",
                  "Mouse click in CREATE_LINE mode at world (%.1f, %.1f) but no station found.",
                  event.worldPosition.x, event.worldPosition.y);
    }
}

void LineCreationSystem::addStationToLine(entt::entity stationEntity) {
    if (_registry.valid(stationEntity) && _registry.all_of<PositionComponent>(stationEntity)) {
        int currentHighestOrder = -1;
        entt::entity lastStationEntity = entt::null;
        auto view = _registry.view<ActiveLineStationTag>();
        for (auto entity : view) {
            const auto &tag = view.get<ActiveLineStationTag>(entity);
            if (tag.order.value > currentHighestOrder) {
                currentHighestOrder = tag.order.value;
                lastStationEntity = entity;
            }
        }

        if (lastStationEntity == stationEntity) {
            LOG_WARN("LineCreationSystem",
                     "Station %u is already the last station in the active line.",
                     static_cast<unsigned int>(stationEntity));
            return;
        }

        _registry.emplace_or_replace<ActiveLineStationTag>(stationEntity,
                                                           StationOrder{currentHighestOrder + 1});
        LOG_DEBUG("LineCreationSystem", "Station %u tagged for active line with order %d.",
                  static_cast<unsigned int>(stationEntity), currentHighestOrder + 1);
    } else {
        LOG_WARN("LineCreationSystem", "Attempted to add invalid station entity: %u",
                 static_cast<unsigned int>(stationEntity));
    }
}

void LineCreationSystem::finalizeLine() {
    std::vector<std::pair<int, entt::entity>> taggedStations;
    auto view = _registry.view<ActiveLineStationTag>();
    for (auto entity : view) {
        taggedStations.push_back({view.get<ActiveLineStationTag>(entity).order.value, entity});
    }

    std::sort(taggedStations.begin(), taggedStations.end());

    if (taggedStations.size() < 2) {
        LOG_WARN("LineCreationSystem",
                 "Not enough stations tagged to finalize line. Need at least 2, have %zu.",
                 taggedStations.size());
        for (const auto &pair : taggedStations) {
            _registry.remove<ActiveLineStationTag>(pair.second);
        }
        return;
    }

    LOG_DEBUG("LineCreationSystem", "Finalizing line with %zu tagged stations.",
              taggedStations.size());

    std::vector<entt::entity> stopsInOrder;
    for (const auto &pair : taggedStations) {
        stopsInOrder.push_back(pair.second);
    }

    sf::Color chosenColor = _colorManager.getNextLineColor();
    entt::entity lineEntity = _entityFactory.createLine(stopsInOrder, chosenColor);

    if (lineEntity == entt::null) {
        LOG_ERROR("LineCreationSystem", "Failed to create line entity.");
        for (entt::entity station_ent : stopsInOrder) {
            if (_registry.valid(station_ent)) {
                _registry.remove<ActiveLineStationTag>(station_ent);
            }
        }
        return;
    }

    for (entt::entity stationEnt : stopsInOrder) {
        if (_registry.valid(stationEnt) && _registry.all_of<StationComponent>(stationEnt)) {
            auto &stationComp = _registry.get<StationComponent>(stationEnt);
            stationComp.connectedLines.push_back(lineEntity);
            LOG_DEBUG("LineCreationSystem", "Connected line %u to station %u",
                      static_cast<unsigned int>(lineEntity), static_cast<unsigned int>(stationEnt));
        } else {
            LOG_WARN("LineCreationSystem",
                     "Station entity %u in line is invalid or missing StationComponent during "
                     "finalization.",
                     static_cast<unsigned int>(stationEnt));
        }
    }

    for (entt::entity stationEnt : stopsInOrder) {
        if (_registry.valid(stationEnt)) {
            _registry.remove<ActiveLineStationTag>(stationEnt);
        }
    }
    LOG_INFO("LineCreationSystem", "Created line entity with ID: %u and removed tags.",
             static_cast<unsigned int>(lineEntity));

    clearCurrentLine();
}

void LineCreationSystem::clearCurrentLine() noexcept {
    LOG_DEBUG("LineCreationSystem",
              "Clearing active line stations (removing ActiveLineStationTag).");
    auto view = _registry.view<ActiveLineStationTag>();
    std::vector<entt::entity> entitiesToClear;
    for (auto entity : view) {
        entitiesToClear.push_back(entity);
    }
    for (auto entity : entitiesToClear) {
        _registry.remove<ActiveLineStationTag>(entity);
    }
    if (!entitiesToClear.empty()) {
        LOG_DEBUG("LineCreationSystem", "Cleared %zu active line station tags.",
                  entitiesToClear.size());
    }
}

void LineCreationSystem::getActiveLineStations(
    std::function<void(entt::entity)> callback) const noexcept {
    auto view = _registry.view<PositionComponent, ActiveLineStationTag>();
    std::vector<std::pair<int, entt::entity>> taggedStations;
    for (auto entity : view) {
        taggedStations.push_back({view.get<ActiveLineStationTag>(entity).order.value, entity});
    }
    std::sort(taggedStations.begin(), taggedStations.end());

    for (const auto &pair : taggedStations) {
        callback(pair.second);
    }
}

void LineCreationSystem::onFinalizeLine(const FinalizeLineEvent &event) {
    LOG_DEBUG("LineCreationSystem", "Processing FinalizeLineEvent.");
    finalizeLine();
}

void LineCreationSystem::update(sf::Time dt) {}