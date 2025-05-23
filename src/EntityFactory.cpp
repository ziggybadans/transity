#include "EntityFactory.h"
#include "Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <vector>

EntityFactory::EntityFactory(entt::registry& registry)
    : m_registry(registry) {
    LOG_INFO("EntityFactory", "EntityFactory created.");
    registerArchetypes();
}

void EntityFactory::registerArchetypes() {
    Archetype stationArchetype;
    stationArchetype.id = "station";

    EntityArchetypeData::RenderableData stationRenderData;
    stationRenderData.radius = 2.0f;
    stationRenderData.color = sf::Color::Blue;
    stationArchetype.renderable_data = stationRenderData;

    EntityArchetypeData::ClickableData stationClickableData;
    if (stationArchetype.renderable_data) {
        stationClickableData.boundingRadius = stationArchetype.renderable_data->radius * 1.5f;
    } else {
        stationClickableData.boundingRadius = 5.0f; // Default value if no renderable data
    }
    stationArchetype.clickable_data = stationClickableData;

    m_archetypes[stationArchetype.id] = stationArchetype;
    LOG_INFO("EntityFactory", "Registered archetype: %s", stationArchetype.id.c_str());
}

void EntityFactory::applyArchetype(entt::entity entity, const Archetype& archetype, const sf::Vector2f& position, const std::string& name) {
    m_registry.emplace<PositionComponent>(entity, position);

    if (archetype.renderable_data) {
        auto& renderable = m_registry.emplace<RenderableComponent>(entity);
        const auto& data = archetype.renderable_data.value();
        renderable.shape.setRadius(data.radius);
        renderable.shape.setFillColor(data.color);
        renderable.shape.setOrigin(sf::Vector2f(data.radius, data.radius));
    }

    if (archetype.clickable_data) {
        auto& clickable = m_registry.emplace<ClickableComponent>(entity);
        clickable.boundingRadius = archetype.clickable_data->boundingRadius;
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

    m_registry.emplace<StationComponent>(entity);
    
    LOG_DEBUG("EntityFactory", "Station entity (ID: %u) created successfully using archetype.", static_cast<unsigned int>(entity));
    return entity;
}

entt::entity EntityFactory::createLine(const std::vector<entt::entity>& stops, const sf::Color& color) {
    LOG_INFO("EntityFactory", "Request to create line entity with %zu stops.", stops.size());
    if (stops.size() < 2) {
        LOG_ERROR("EntityFactory", "Cannot create line with less than 2 stops.");
        return entt::null;
    }
    
    auto entity = m_registry.create();
    auto& lineComponent = m_registry.emplace<LineComponent>(entity);
    lineComponent.stops = stops;
    lineComponent.color = color;

    LOG_DEBUG("EntityFactory", "Line entity (ID: %u) created successfully with %zu stops.", static_cast<unsigned int>(entity), stops.size());
    return entity;
}