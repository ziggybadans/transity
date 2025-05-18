#include "EntityFactory.h"
#include "Logger.h"
#include <SFML/Graphics/Color.hpp>

EntityFactory::EntityFactory(entt::registry& registry)
    : m_registry(registry) {
    LOG_INFO("EntityFactory", "EntityFactory created.");
}

entt::entity EntityFactory::createStation(const sf::Vector2f& position, const std::string& name) {
    LOG_INFO("EntityFactory", "Creating station entity with name '%s' at (%.1f, %.1f).", name.c_str(), position.x, position.y);
    if (name.empty()) {
        LOG_WARN("EntityFactory", "Creating station entity with an empty name at (%.1f, %.1f).", position.x, position.y);
    }
    auto entity = m_registry.create();

    m_registry.emplace<PositionComponent>(entity, position);
    m_registry.emplace<NameComponent>(entity, name);
    m_registry.emplace<StationTag>(entity);

    auto& renderable = m_registry.emplace<RenderableComponent>(entity);
    
    const float stationRadius = 2.f;
    renderable.shape.setRadius(stationRadius);
    renderable.shape.setFillColor(sf::Color::Blue);
    renderable.shape.setOrigin(stationRadius, stationRadius);
    LOG_DEBUG("EntityFactory", "Station entity (ID: %u) created successfully.", static_cast<unsigned int>(entity));
    return entity;
}