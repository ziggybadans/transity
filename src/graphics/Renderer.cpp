#include "Renderer.h"
#include "../core/Components.h"
#include "../Logger.h"
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>

Renderer::Renderer()
    : _windowInstance(sf::VideoMode({2160, 3840}), "Transity Predev")
    , _clearColor(173, 216, 230) {
    LOG_INFO("Renderer", "Renderer created and window initialized.");
    _windowInstance.setFramerateLimit(144);
}

Renderer::~Renderer() {
    LOG_INFO("Renderer", "Renderer destroyed.");
}

void Renderer::initialize() {
    LOG_INFO("Renderer", "Initializing Renderer.");
    LOG_INFO("Renderer", "Renderer initialized.");
}

void Renderer::renderFrame(entt::registry& registry, 
    const sf::View& view, 
    sf::Time dt, 
    InteractionMode currentMode,
    bool visualizeNoise) {
    LOG_TRACE("Renderer", "Beginning render pass.");
    _windowInstance.setView(view);
    _windowInstance.clear(_clearColor);

    _terrainRenderSystem.setVisualizeNoise(visualizeNoise);

    _terrainRenderSystem.render(registry, _windowInstance);
    LOG_TRACE("Renderer", "Terrain rendered.");

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

            const auto& pos1Comp = registry.get<PositionComponent>(station1);
            const auto& pos2Comp = registry.get<PositionComponent>(station2);

            sf::Vertex line[] = {
                sf::Vertex{pos1Comp.coordinates, lineComp.color, sf::Vector2f()},
                sf::Vertex{pos2Comp.coordinates, lineComp.color, sf::Vector2f()}
            };
            _windowInstance.draw(line, 2, sf::PrimitiveType::Lines);
        }
        finalizedLineCount++;
    }
    if (finalizedLineCount > 0) {
        LOG_TRACE("Renderer", "Rendered %d finalized lines.", finalizedLineCount);
    }

    if (currentMode == InteractionMode::CREATE_LINE) {
        std::vector<std::pair<int, entt::entity>> taggedStationsPairs;
        auto activeStationsView = registry.view<PositionComponent, ActiveLineStationTag>(); // Assuming ActiveLineStationTag is the name
        for (auto entity : activeStationsView) {
            const auto& tag = activeStationsView.get<ActiveLineStationTag>(entity);
            taggedStationsPairs.push_back({tag.order, entity});
        }

        if (!taggedStationsPairs.empty()) {
            LOG_TRACE("Renderer", "Found %zu stations tagged for active line.", taggedStationsPairs.size());
            std::sort(taggedStationsPairs.begin(), taggedStationsPairs.end());

            std::vector<entt::entity> activeLineStationsInOrder;
            for (const auto& pair : taggedStationsPairs) {
                activeLineStationsInOrder.push_back(pair.second);
            }
            if (activeLineStationsInOrder.size() >= 2) {
                for (size_t i = 0; i < activeLineStationsInOrder.size() - 1; ++i) {
                    entt::entity stationEntity1 = activeLineStationsInOrder[i];
                    entt::entity stationEntity2 = activeLineStationsInOrder[i+1];

                    if (!registry.valid(stationEntity1) || !registry.all_of<PositionComponent>(stationEntity1) ||
                        !registry.valid(stationEntity2) || !registry.all_of<PositionComponent>(stationEntity2)) {
                        LOG_WARN("Renderer", "Skipping active line segment due to invalid station or missing PositionComponent.");
                        continue;
                    }
                    const auto& pos1Comp = registry.get<PositionComponent>(stationEntity1);
                    const auto& pos2Comp = registry.get<PositionComponent>(stationEntity2);

                    sf::Vertex lineSegment[] = { // Use a different variable name to avoid conflict
                        sf::Vertex{pos1Comp.coordinates, sf::Color::Yellow, sf::Vector2f()}, // Changed to Yellow for active line
                        sf::Vertex{pos2Comp.coordinates, sf::Color::Yellow, sf::Vector2f()}
                    };
                    _windowInstance.draw(lineSegment, 2, sf::PrimitiveType::Lines);
                    LOG_TRACE("Renderer", "Drew active line segment between selected stations.");
                }
            }
            if (!activeLineStationsInOrder.empty()) {
                const auto& lastStationPosComp = registry.get<PositionComponent>(activeLineStationsInOrder.back());
                sf::Vector2f p1 = lastStationPosComp.coordinates;
                
                sf::Vector2i currentMousePixelPos = sf::Mouse::getPosition(_windowInstance); // Get current mouse pos
                sf::Vector2f p2Mouse = _windowInstance.mapPixelToCoords(currentMousePixelPos, view); // Use the view passed to render

                sf::Vertex lineToMouse[] = {
                    sf::Vertex{p1, sf::Color::Yellow, sf::Vector2f()},
                    sf::Vertex{p2Mouse, sf::Color::Yellow, sf::Vector2f()}
                };
                _windowInstance.draw(lineToMouse, 2, sf::PrimitiveType::Lines);
                LOG_TRACE("Renderer", "Drew active line segment from last station to mouse.");
            }
        }
    }

    auto viewRegistry = registry.view<PositionComponent, RenderableComponent>();
    int entityCount = 0;
    for (auto entity : viewRegistry) {
        auto& position = viewRegistry.get<PositionComponent>(entity);
        auto& renderable = viewRegistry.get<RenderableComponent>(entity);

        renderable.shape.setPosition(position.coordinates);
        _windowInstance.draw(renderable.shape);
        entityCount++;
    }
    LOG_TRACE("Renderer", "Rendered %d entities.", entityCount);

    LOG_TRACE("Renderer", "Render pass complete.");
}

void Renderer::displayFrame() {
    _windowInstance.display();
}

void Renderer::cleanupResources() {
    LOG_INFO("Renderer", "Renderer cleanup initiated.");
    LOG_INFO("Renderer", "Renderer cleaned up.");
}

bool Renderer::isWindowOpen() const {
    return _windowInstance.isOpen();
}

sf::RenderWindow& Renderer::getWindowInstance(){
    return _windowInstance;
}

void Renderer::setClearColor(const sf::Color& color) {
    _clearColor = color;
    LOG_DEBUG("Renderer", "Clear color set to R:%d G:%d B:%d A:%d", color.r, color.g, color.b, color.a);
}

const sf::Color& Renderer::getClearColor() const {
    return _clearColor;
}