#include "Renderer.h"
#include "Components.h" // Still needed for PositionComponent, RenderableComponent
#include "Logger.h"
// #include "imgui.h" // Removed
// #include "imgui-SFML.h" // Removed
#include <entt/entt.hpp> // Still needed for registry.view
#include <SFML/Graphics.hpp>
#include <iostream> // Keep for now
#include <cstdlib> // For exit()

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
    LOG_INFO("Renderer", "Initializing Renderer."); // ImGui init removed
    m_landShape.setSize({100, 100});
    m_landShape.setFillColor(sf::Color::White);
    m_landShape.setOrigin(m_landShape.getSize() / 2.0f);
    m_landShape.setPosition({50, 50});
    LOG_DEBUG("Renderer", "Land shape created at (%.1f, %.1f) with size (%.1f, %.1f).", m_landShape.getPosition().x, m_landShape.getPosition().y, m_landShape.getSize().x, m_landShape.getSize().y);

    // ImGui::CreateContext(); // Removed
    // if (!ImGui::SFML::Init(m_windowInstance)) { // Removed
    //     LOG_FATAL("Renderer", "Failed to initialize ImGui-SFML"); // Removed
    //     exit(EXIT_FAILURE); // Removed
    // } // Removed
    // ImGui::StyleColorsDark(); // Removed
    LOG_INFO("Renderer", "Renderer initialized."); // ImGui part removed
}

void Renderer::render(entt::registry& registry, const sf::View& view, sf::Time dt) {
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

    LOG_TRACE("Renderer", "Render pass complete.");
}

// void Renderer::updateImGui(sf::Time dt, InteractionMode& currentMode) { // Method removed
// }

// void Renderer::renderImGui() { // Method removed
// }

void Renderer::display() {
    m_windowInstance.display();
}

void Renderer::cleanup() {
    LOG_INFO("Renderer", "Renderer cleanup initiated.");
    // ImGui::SFML::Shutdown(m_windowInstance); // Removed, UIManager handles ImGui shutdown
    LOG_INFO("Renderer", "Renderer cleaned up."); // ImGui part removed
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