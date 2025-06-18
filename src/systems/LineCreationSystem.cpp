#include "LineCreationSystem.h"
#include "../core/Components.h"
#include "../core/ServiceLocator.h"
#include "../core/EntityFactory.h"
#include "../graphics/ColorManager.h"
#include "../Logger.h"
#include <algorithm>
#include <utility>

LineCreationSystem::LineCreationSystem(ServiceLocator& serviceLocator)
    : _registry(serviceLocator.registry),
      _entityFactory(serviceLocator.entityFactory),
      _colorManager(serviceLocator.colorManager) {
    m_addStationConnection = serviceLocator.eventBus->sink<AddStationToLineEvent>().connect<&LineCreationSystem::onAddStationToLine>(this);
    m_finalizeLineConnection = serviceLocator.eventBus->sink<FinalizeLineEvent>().connect<&LineCreationSystem::onFinalizeLine>(this);
    LOG_INFO("LineCreationSystem", "LineCreationSystem created and connected to EventBus.");
}

LineCreationSystem::~LineCreationSystem() {
    m_addStationConnection.release();
    m_finalizeLineConnection.release();
    LOG_INFO("LineCreationSystem", "LineCreationSystem destroyed and disconnected from EventBus.");
}

void LineCreationSystem::addStationToLine(entt::entity stationEntity) {
    if (_registry->valid(stationEntity) && _registry->all_of<PositionComponent>(stationEntity)) {
        int currentHighestOrder = -1;
        entt::entity lastStationEntity = entt::null;
        auto view = _registry->view<ActiveLineStationTag>();
        for (auto entity : view) {
            const auto& tag = view.get<ActiveLineStationTag>(entity);
            if (tag.order > currentHighestOrder) {
                currentHighestOrder = tag.order;
                lastStationEntity = entity;
            }
        }

        if (lastStationEntity == stationEntity) {
            LOG_WARN("LineCreationSystem", "Station %u is already the last station in the active line.", static_cast<unsigned int>(stationEntity));
            return;
        }

        _registry->emplace_or_replace<ActiveLineStationTag>(stationEntity, currentHighestOrder + 1);
        LOG_DEBUG("LineCreationSystem", "Station %u tagged for active line with order %d.", static_cast<unsigned int>(stationEntity), currentHighestOrder + 1);
    } else {
        LOG_WARN("LineCreationSystem", "Attempted to add invalid station entity: %u", static_cast<unsigned int>(stationEntity));
    }
}

void LineCreationSystem::finalizeLine() {
    std::vector<std::pair<int, entt::entity>> taggedStations;
    auto view = _registry->view<ActiveLineStationTag>();
    for (auto entity : view) {
        taggedStations.push_back({view.get<ActiveLineStationTag>(entity).order, entity});
    }

    std::sort(taggedStations.begin(), taggedStations.end());

    if (taggedStations.size() < 2) {
        LOG_WARN("LineCreationSystem", "Not enough stations tagged to finalize line. Need at least 2, have %zu.", taggedStations.size());
        for (const auto& pair : taggedStations) {
            _registry->remove<ActiveLineStationTag>(pair.second);
        }
        return;
    }

    LOG_DEBUG("LineCreationSystem", "Finalizing line with %zu tagged stations.", taggedStations.size());

    std::vector<entt::entity> stopsInOrder;
    for (const auto& pair : taggedStations) {
        stopsInOrder.push_back(pair.second);
    }

    sf::Color chosenColor = _colorManager->getNextLineColor();
    entt::entity lineEntity = _entityFactory->createLine(stopsInOrder, chosenColor);

    if (lineEntity == entt::null) {
        LOG_ERROR("LineCreationSystem", "Failed to create line entity.");
        for (entt::entity station_ent : stopsInOrder) {
            if (_registry->valid(station_ent)) {
                _registry->remove<ActiveLineStationTag>(station_ent);
            }
        }
        return;
    }

    for (entt::entity stationEnt : stopsInOrder) {
        if (_registry->valid(stationEnt) && _registry->all_of<StationComponent>(stationEnt)) {
            auto& stationComp = _registry->get<StationComponent>(stationEnt);
            stationComp.connectedLines.push_back(lineEntity);
            LOG_DEBUG("LineCreationSystem", "Connected line %u to station %u", static_cast<unsigned int>(lineEntity), static_cast<unsigned int>(stationEnt));
        } else {
            LOG_WARN("LineCreationSystem", "Station entity %u in line is invalid or missing StationComponent during finalization.", static_cast<unsigned int>(stationEnt));
        }
    }

    for (entt::entity stationEnt : stopsInOrder) {
        if (_registry->valid(stationEnt)) {
            _registry->remove<ActiveLineStationTag>(stationEnt);
        }
    }
    LOG_INFO("LineCreationSystem", "Created line entity with ID: %u and removed tags.", static_cast<unsigned int>(lineEntity));
    
    clearCurrentLine();
}

void LineCreationSystem::clearCurrentLine() {
    LOG_DEBUG("LineCreationSystem", "Clearing active line stations (removing ActiveLineStationTag).");
    auto view = _registry->view<ActiveLineStationTag>();
    std::vector<entt::entity> entitiesToClear;
    for (auto entity : view) {
        entitiesToClear.push_back(entity);
    }
    for (auto entity : entitiesToClear) {
        _registry->remove<ActiveLineStationTag>(entity);
    }
    if (!entitiesToClear.empty()) {
        LOG_DEBUG("LineCreationSystem", "Cleared %zu active line station tags.", entitiesToClear.size());
    }
}

std::vector<entt::entity> LineCreationSystem::getActiveLineStations() const {
    std::vector<std::pair<int, entt::entity>> taggedStations;
    auto view = _registry->view<PositionComponent, ActiveLineStationTag>();
    for (auto entity : view) {
        taggedStations.push_back({view.get<ActiveLineStationTag>(entity).order, entity});
    }
    std::sort(taggedStations.begin(), taggedStations.end());

    std::vector<entt::entity> stationsInOrder;
    for (const auto& pair : taggedStations) {
        stationsInOrder.push_back(pair.second);
    }
    return stationsInOrder;
}

void LineCreationSystem::onAddStationToLine(const AddStationToLineEvent& event) {
    LOG_DEBUG("LineCreationSystem", "Processing AddStationToLineEvent for station %u.", static_cast<unsigned int>(event.stationEntity));
    addStationToLine(event.stationEntity);
}

void LineCreationSystem::onFinalizeLine(const FinalizeLineEvent& event) {
    LOG_DEBUG("LineCreationSystem", "Processing FinalizeLineEvent.");
    finalizeLine();
}
