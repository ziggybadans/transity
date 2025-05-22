#include "Renderer.h"
#include "Components.h"
#include "Logger.h"
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>

Renderer::Renderer()
    : m_windowInstance(sf::VideoMode({800, 600}), "Transity Predev")
    , m_clearColor(173, 216, 230) {
    LOG_INFO("Renderer", "Renderer created and window initialized.");
    m_windowInstance.setFramerateLimit(144);
}

Renderer::~Renderer() {
    LOG_INFO("Renderer", "Renderer destroyed.");
}

void Renderer::init() {
    LOG_INFO("Renderer", "Initializing Renderer.");
    m_landShape.setSize({100, 100});
    m_landShape.setFillColor(sf::Color::White);
    m_landShape.setOrigin(m_landShape.getSize() / 2.0f);
    m_landShape.setPosition({50, 50});
    LOG_DEBUG("Renderer", "Land shape created at (%.1f, %.1f) with size (%.1f, %.1f).", m_landShape.getPosition().x, m_landShape.getPosition().y, m_landShape.getSize().x, m_landShape.getSize().y);
    LOG_INFO("Renderer", "Renderer initialized.");
}

void Renderer::render(entt::registry& registry, const sf::View& view, sf::Time dt, const std::vector<entt::entity>& activeLineStations, InteractionMode currentMode) {
    LOG_TRACE("Renderer", "Beginning render pass.");
    m_windowInstance.setView(view);
    m_windowInstance.clear(m_clearColor);

    m_windowInstance.draw(m_landShape);
    LOG_TRACE("Renderer", "Land shape drawn.");

    auto view_registry = registry.view<PositionComponent, RenderableComponent>();
    int entityCount = 0;
    for (auto entity : view_registry) {
        auto& position = view_registry.get<PositionComponent>(entity);
        auto& renderable = view_registry.get<RenderableComponent>(entity);

        renderable.shape.setPosition(position.coordinates);
        m_windowInstance.draw(renderable.shape);
        entityCount++;
    }
    LOG_TRACE("Renderer", "Rendered %d entities.", entityCount);

    LOG_TRACE("Renderer", "Rendering finalized lines.");
    auto lineView = registry.view<LineComponent>();
    int finalizedLineCount = 0;
    for (auto entity : lineView) {
        const auto& lineComp = lineView.get<LineComponent>(entity);
        if (lineComp.stops.size() < 2) {
            LOG_WARN("Renderer", "Line has less than 2 stops, skipping rendering.");
            continue;
        }

        for (size_t i = 0; i < lineComp.stops.size() - 1; ++i) {
            entt::entity station1 = lineComp.stops[i];
            entt::entity station2 = lineComp.stops[i + 1];

            if (!registry.valid(station1) || !registry.all_of<PositionComponent>(station1) || 
                !registry.valid(station2) || !registry.all_of<PositionComponent>(station2)) {
                LOG_WARN("Renderer", "Invalid entity in line stops.");
                continue;
            }

            const auto& pos1_comp = registry.get<PositionComponent>(station1);
            const auto& pos2_comp = registry.get<PositionComponent>(station2);

            sf::Vertex line[] = {
                sf::Vertex{pos1_comp.coordinates, lineComp.color, sf::Vector2f()},
                sf::Vertex{pos2_comp.coordinates, lineComp.color, sf::Vector2f()}
            };
            m_windowInstance.draw(line, 2, sf::PrimitiveType::Lines);
        }
        finalizedLineCount++;
    }
    if (finalizedLineCount > 0) {
        LOG_TRACE("Renderer", "Rendered %d finalized lines.", finalizedLineCount);
    }

    if (currentMode == InteractionMode::CREATE_LINE && activeLineStations.size() >= 1) {
        LOG_TRACE("Renderer", "Drawing active line with %zu stations.", activeLineStations.size());

        if (activeLineStations.size() >= 2) { // Need at least two points to draw a segment
            for (size_t i = 0; i < activeLineStations.size() - 1; ++i) {
                entt::entity station_entity1 = activeLineStations[i];
                entt::entity station_entity2 = activeLineStations[i+1];

                if (!registry.valid(station_entity1) || !registry.all_of<PositionComponent>(station_entity1) ||
                    !registry.valid(station_entity2) || !registry.all_of<PositionComponent>(station_entity2)) {
                    LOG_WARN("Renderer", "Skipping active line segment due to invalid station or missing PositionComponent.");
                    continue;
                }
                const auto& pos1_comp = registry.get<PositionComponent>(station_entity1);
                const auto& pos2_comp = registry.get<PositionComponent>(station_entity2);

                sf::Vertex line_segment[] = { // Use a different variable name to avoid conflict
                    sf::Vertex{pos1_comp.coordinates, sf::Color::Yellow, sf::Vector2f()}, // Changed to Yellow for active line
                    sf::Vertex{pos2_comp.coordinates, sf::Color::Yellow, sf::Vector2f()}
                };
                m_windowInstance.draw(line_segment, 2, sf::PrimitiveType::Lines);
                LOG_TRACE("Renderer", "Drew active line segment between selected stations.");
            }
        }

        // For drawing from the LAST selected station to the CURRENT mouse cursor:
        if (!activeLineStations.empty()) {
            if (registry.valid(activeLineStations.back()) && registry.all_of<PositionComponent>(activeLineStations.back())) {
                const auto& lastStationPosComp = registry.get<PositionComponent>(activeLineStations.back());
                sf::Vector2f p1 = lastStationPosComp.coordinates;
                
                sf::Vector2i currentMousePixelPos = sf::Mouse::getPosition(m_windowInstance); // Get current mouse pos
                sf::Vector2f p2_mouse = m_windowInstance.mapPixelToCoords(currentMousePixelPos, view); // Use the view passed to render

                sf::Vertex lineToMouse[] = {
                    sf::Vertex{p1, sf::Color::Yellow, sf::Vector2f()},
                    sf::Vertex{p2_mouse, sf::Color::Yellow, sf::Vector2f()}
                };
                m_windowInstance.draw(lineToMouse, 2, sf::PrimitiveType::Lines);
                LOG_TRACE("Renderer", "Drew active line segment from last station to mouse.");
            }
        }
    }

    LOG_TRACE("Renderer", "Render pass complete.");
}

void Renderer::display() {
    m_windowInstance.display();
}

void Renderer::cleanup() {
    LOG_INFO("Renderer", "Renderer cleanup initiated.");
    LOG_INFO("Renderer", "Renderer cleaned up.");
}

bool Renderer::isOpen() const {
    return m_windowInstance.isOpen();
}

sf::RenderWindow& Renderer::getWindow(){
    return m_windowInstance;
}

void Renderer::setClearColor(const sf::Color& color) {
    m_clearColor = color;
    LOG_DEBUG("Renderer", "Clear color set to R:%d G:%d B:%d A:%d", color.r, color.g, color.b, color.a);
}

const sf::Color& Renderer::getClearColor() const {
    return m_clearColor;
}

sf::Vector2f Renderer::getLandCenter() const {
    LOG_TRACE("Renderer", "Getting land center.");
    return m_landShape.getPosition();
}

sf::Vector2f Renderer::getLandSize() const {
    LOG_TRACE("Renderer", "Getting land size.");
    return m_landShape.getSize();
}