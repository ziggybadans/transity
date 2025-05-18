#include "EntityFactory.h"

#include <SFML/Graphics/Color.hpp>

EntityFactory::EntityFactory(entt::registry& registry)
    : m_registry(registry) {
}

entt::entity EntityFactory::createStation(const sf::Vector2f& position, const std::string& name) {
    auto entity = m_registry.create();

    m_registry.emplace<PositionComponent>(entity, position);
    m_registry.emplace<NameComponent>(entity, name);
    m_registry.emplace<StationTag>(entity);

    auto& renderable = m_registry.emplace<RenderableComponent>(entity);
    
    const float stationRadius = 2.f;
    renderable.shape.setRadius(stationRadius);
    renderable.shape.setFillColor(sf::Color::Blue);
    renderable.shape.setOrigin(stationRadius, stationRadius);

    return entity;
}