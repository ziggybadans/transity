#include "EntityFactory.h"
#include "Logger.h"
#include <SFML/Graphics/Color.hpp>

EntityFactory::EntityFactory(entt::registry& registry)
    : m_registry(registry) {
    LOG_INFO("EntityFactory", "EntityFactory created.");
    registerArchetypes();
}

void EntityFactory::registerArchetypes() {
    Archetype stationArchetype;
    stationArchetype.id = "station";
    stationArchetype.has_station_tag = true;
    EntityArchetypeData::RenderableData stationRenderData;
    stationRenderData.radius = 2.0f;
    stationRenderData.color = sf::Color::Blue;
    stationArchetype.renderable_data = stationRenderData;
    m_archetypes[stationArchetype.id] = stationArchetype;
    LOG_INFO("EntityFactory", "Registered archetype: %s", stationArchetype.id.c_str());
}

void EntityFactory::applyArchetype(entt::entity entity, const Archetype& archetype, const sf::Vector2f& position, const std::string& name) {
    m_registry.emplace<PositionComponent>(entity, position);
    if (!name.empty()) {
        m_registry.emplace<NameComponent>(entity, name);
    } else {
        LOG_WARN("EntityFactory", "Creating entity of type '%s' with an empty name at (%.1f, %.1f).", archetype.id.c_str(), position.x, position.y);
    }

    if (archetype.has_station_tag) {
        m_registry.emplace<StationTag>(entity);
    }

    if (archetype.renderable_data) {
        auto& renderable = m_registry.emplace<RenderableComponent>(entity);
        const auto& data = archetype.renderable_data.value();
        renderable.shape.setRadius(data.radius);
        renderable.shape.setFillColor(data.color);
        renderable.shape.setOrigin(sf::Vector2f(data.radius, data.radius));
    }
    LOG_DEBUG("EntityFactory", "Applied archetype '%s' to entity (ID: %u).", archetype.id.c_str(), static_cast<unsigned int>(entity));
}

entt::entity EntityFactory::createStation(const sf::Vector2f& position, const std::string& name) {
    LOG_INFO("EntityFactory", "Request to create station entity with name '%s' at (%.1f, %.1f).", name.c_str(), position.x, position.y);
    
    auto it = m_archetypes.find("station");
    if (it == m_archetypes.end()) {
        LOG_ERROR("EntityFactory", "Archetype 'station' not found. Cannot create station entity.");
        return entt::null;
    }

    const Archetype& stationArchetype = it->second;
    auto entity = m_registry.create();
    applyArchetype(entity, stationArchetype, position, name);
    
    LOG_DEBUG("EntityFactory", "Station entity (ID: %u) created successfully using archetype.", static_cast<unsigned int>(entity));
    return entity;
}