#include "LineCreationSystem.h"
#include "Components.h"
#include "Logger.h"
#include <algorithm>
#include <utility>
#include <variant>
#include <type_traits>

LineCreationSystem::LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory)
    : m_registry(registry), m_entityFactory(entityFactory) {
    LOG_INFO("LineCreationSystem", "LineCreationSystem created.");
}

void LineCreationSystem::addStationToLine(entt::entity stationEntity) {
    // In LineCreationSystem::addStationToLine
    if (m_registry.valid(stationEntity) && m_registry.all_of<PositionComponent>(stationEntity)) {
        // Check if it's already the last station (has the highest order tag)
        int currentHighestOrder = -1;
        entt::entity lastStationEntity = entt::null;
        auto view = m_registry.view<ActiveLineStationTag>();
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

        // If not, add or update the tag with the next order
        m_registry.emplace_or_replace<ActiveLineStationTag>(stationEntity, currentHighestOrder + 1);
        LOG_DEBUG("LineCreationSystem", "Station %u tagged for active line with order %d.", static_cast<unsigned int>(stationEntity), currentHighestOrder + 1);
    } else {
        LOG_WARN("LineCreationSystem", "Attempted to add invalid station entity: %u", static_cast<unsigned int>(stationEntity));
    }
}

void LineCreationSystem::finalizeLine() {
    // In LineCreationSystem::finalizeLine
    std::vector<std::pair<int, entt::entity>> taggedStations;
    auto view = m_registry.view<ActiveLineStationTag>();
    for (auto entity : view) {
        taggedStations.push_back({view.get<ActiveLineStationTag>(entity).order, entity});
    }

    std::sort(taggedStations.begin(), taggedStations.end()); // Sort by order

    if (taggedStations.size() < 2) {
        LOG_WARN("LineCreationSystem", "Not enough stations tagged to finalize line. Need at least 2, have %zu.", taggedStations.size());
        // Also clear any stray tags if desired, though clearCurrentLine will do it
        for (const auto& pair : taggedStations) {
            m_registry.remove<ActiveLineStationTag>(pair.second);
        }
        return;
    }

    LOG_DEBUG("LineCreationSystem", "Finalizing line with %zu tagged stations.", taggedStations.size());

    std::vector<entt::entity> stopsInOrder;
    for (const auto& pair : taggedStations) {
        stopsInOrder.push_back(pair.second);
    }

    // Color cycling logic (can be kept here or moved to EntityFactory later)
    static int lineColorIndex = 0;
    sf::Color lineColors[] = { sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan };
    sf::Color chosenColor = lineColors[lineColorIndex % (sizeof(lineColors) / sizeof(lineColors[0]))];
    lineColorIndex++;

    entt::entity lineEntity = m_entityFactory.createLine(stopsInOrder, chosenColor);

    if (lineEntity == entt::null) {
        LOG_ERROR("LineCreationSystem", "Failed to create line entity.");
        for (entt::entity station_ent : stopsInOrder) {
            if (m_registry.valid(station_ent)) {
                m_registry.remove<ActiveLineStationTag>(station_ent); // Clean up tags
            }
        }
        return;
    }

    for (entt::entity station_ent : stopsInOrder) {
        if (m_registry.valid(station_ent) && m_registry.all_of<StationComponent>(station_ent)) {
            auto& stationComp = m_registry.get<StationComponent>(station_ent);
            stationComp.connectedLines.push_back(lineEntity);
            LOG_DEBUG("LineCreationSystem", "Connected line %u to station %u", static_cast<unsigned int>(lineEntity), static_cast<unsigned int>(station_ent));
        } else {
            LOG_WARN("LineCreationSystem", "Station entity %u in line is invalid or missing StationComponent during finalization.", static_cast<unsigned int>(station_ent));
        }
    }

    // After successful line creation, remove tags
    for (entt::entity station_ent : stopsInOrder) {
        if (m_registry.valid(station_ent)) { // Check validity before removing
            m_registry.remove<ActiveLineStationTag>(station_ent);
        }
    }
    LOG_INFO("LineCreationSystem", "Created line entity with ID: %u and removed tags.", static_cast<unsigned int>(lineEntity));
    // No need to call clearCurrentLine() if it only dealt with m_stationsForCurrentLine
    // The equivalent is now removing the tags, which is done above.
    
    clearCurrentLine(); // This now calls the member function
}

void LineCreationSystem::clearCurrentLine() {
    // In LineCreationSystem::clearCurrentLine
    LOG_DEBUG("LineCreationSystem", "Clearing active line stations (removing ActiveLineStationTag).");
    auto view = m_registry.view<ActiveLineStationTag>();
    // Create a list of entities to remove components from, to avoid iterator invalidation issues
    // if on_destroy<ActiveLineStationTag> or similar were to modify the view.
    std::vector<entt::entity> entitiesToClear;
    for (auto entity : view) {
        entitiesToClear.push_back(entity);
    }
    for (auto entity : entitiesToClear) {
        m_registry.remove<ActiveLineStationTag>(entity);
    }
    if (!entitiesToClear.empty()) {
        LOG_DEBUG("LineCreationSystem", "Cleared %zu active line station tags.", entitiesToClear.size());
    }
}

std::vector<entt::entity> LineCreationSystem::getActiveLineStations() const {
    // In LineCreationSystem::getActiveLineStations
    std::vector<std::pair<int, entt::entity>> taggedStations;
    auto view = m_registry.view<PositionComponent, ActiveLineStationTag>(); // Ensure they also have Position
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

void LineCreationSystem::processEvents(
    const std::vector<std::variant<AddStationToLineEvent, FinalizeLineEvent>>& inputHandlerEvents,
    const std::vector<FinalizeLineEvent>& uiEvents) {
    
    for (const auto& eventVariant : inputHandlerEvents) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, AddStationToLineEvent>) {
                LOG_DEBUG("LineCreationSystem", "Processing AddStationToLineEvent for station %u.", static_cast<unsigned int>(arg.stationEntity));
                addStationToLine(arg.stationEntity);
            }
            // Add other event types from InputHandler if LineCreationSystem cares about them
        }, eventVariant);
    }

    for (const auto& uiEvent : uiEvents) {
        // Since uiEvents only contains FinalizeLineEvent for now, we can directly process.
        // If it were a variant, you'd use std::visit here too.
        LOG_DEBUG("LineCreationSystem", "Processing FinalizeLineEvent from UI.");
        finalizeLine();
    }
}