#include "EntityFactory.h" // This includes Components.h, entt/entt.hpp, sf::Vector2f, std::string

// We use sf::Color directly, so include its header.
// Components.h should handle the include for sf::CircleShape if RenderableComponent::shape is one.
#include <SFML/Graphics/Color.hpp>

EntityFactory::EntityFactory(entt::registry& registry)
    : m_registry(registry) {
    // Constructor implementation (initialization list is sufficient)
}

entt::entity EntityFactory::createStation(const sf::Vector2f& position, const std::string& name) {
    auto entity = m_registry.create();

    // Add PositionComponent, assuming it can be constructed from sf::Vector2f
    m_registry.emplace<PositionComponent>(entity, position);

    // Add NameComponent
    m_registry.emplace<NameComponent>(entity, name);

    // Add StationTag (default constructed)
    m_registry.emplace<StationTag>(entity);

    // Add and configure RenderableComponent
    // This assumes RenderableComponent has a member, e.g., `sf::CircleShape shape;`
    // as suggested by "Configuring the RenderableComponent's shape (e.g., sf::CircleShape),
    // setting its radius..."
    auto& renderable = m_registry.emplace<RenderableComponent>(entity);
    
    const float stationRadius = 10.f; // As per "radius (e.g., to 10.f)"
    renderable.shape.setRadius(stationRadius);
    renderable.shape.setFillColor(sf::Color::Blue); // As per "fill color (e.g., sf::Color::Blue)"
    renderable.shape.setOrigin(stationRadius, stationRadius); // As per "origin (e.g., to the center: radius, radius)"

    return entity;
}