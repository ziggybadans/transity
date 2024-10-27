#include "Renderer.h"
#include <iostream>
#include <algorithm>

Renderer::Renderer() {}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& /*window*/) {
    isInitialized = true;

    if (!font.loadFromFile("assets/PTSans-Regular.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
        return false;
    }

    // Initialize hoveredCityText
    hoveredCityText.setFont(font);
    hoveredCityText.setCharacterSize(28); // Adjust as needed
    hoveredCityText.setFillColor(sf::Color::Black);

    return true;
}

void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    worldMap = map;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    std::lock_guard<std::mutex> lock(renderMutex);

    renderWorldMap(window, camera);
    renderCities(window, camera);
    renderHoveredCityName(window);
}

void Renderer::Shutdown() {
    isInitialized = false;
    worldMap.reset();
}

void Renderer::renderWorldMap(sf::RenderWindow& window, const Camera& camera) {
    if (worldMap) {
        worldMap->Render(window, camera);
    }
}

void Renderer::renderCities(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& cities = worldMap->GetCities();
    float currentZoom = camera.GetZoomLevel();

    // Set up circle shape
    sf::CircleShape circle;
    circle.setFillColor(sf::Color::White);
    circle.setOutlineColor(sf::Color::Black);

    // Apply camera view
    sf::View originalView = window.getView();
    window.setView(camera.GetView());

    // Get mouse position in world coordinates
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);

    // Reset hoveredCityName
    hoveredCityName.clear();

    for (const auto& city : cities) {
        bool shouldRender = false;
        float citySizeFactor = 1.0f;

        switch (city.category) {
        case PlaceCategory::CapitalCity:
            shouldRender = true;
            citySizeFactor = 1.0f;
            break;
        case PlaceCategory::City:
            if (currentZoom <= 0.1f) {
                shouldRender = true;
                citySizeFactor = 0.75f;
            }
            break;
        case PlaceCategory::Town:
            if (currentZoom <= 0.01f) {
                shouldRender = true;
                citySizeFactor = 0.5f;
            }
            break;
        case PlaceCategory::Suburb:
            if (currentZoom <= 0.001f) {
                shouldRender = true;
                citySizeFactor = 0.25f;
            }
            break;
        default:
            break;
        }

        if (!shouldRender) continue;

        // Calculate scaled circle radius
        float scaledCircleRadius = std::min(BASE_CIRCLE_RADIUS * currentZoom * citySizeFactor + (8 * currentZoom), MAX_CIRCLE_RADIUS);

        // Update circle properties
        circle.setRadius(scaledCircleRadius);
        circle.setOrigin(scaledCircleRadius, scaledCircleRadius);
        circle.setOutlineThickness(std::min(4.0f * currentZoom, 4.0f));
        circle.setPosition(city.position);

        // Update hovered city if necessary
        updateHoveredCity(mouseWorldPos, city, scaledCircleRadius, circle);

        window.draw(circle);

        // Reset circle color for next iteration
        circle.setFillColor(sf::Color::White);
    }

    // Restore the original view
    window.setView(originalView);
}

void Renderer::updateHoveredCity(const sf::Vector2f& mouseWorldPos, const City& city, float scaledCircleRadius, sf::CircleShape& circle) {
    float dx = mouseWorldPos.x - city.position.x;
    float dy = mouseWorldPos.y - city.position.y;
    float distanceSquared = dx * dx + dy * dy;

    if (distanceSquared <= scaledCircleRadius * scaledCircleRadius) {
        hoveredCityName = city.name;
        circle.setFillColor(sf::Color::Yellow); // Highlight color
    }
}

void Renderer::renderHoveredCityName(sf::RenderWindow& window) {
    if (hoveredCityName.empty()) return;

    hoveredCityText.setString(hoveredCityName);

    // Get window size
    sf::Vector2u windowSize = window.getSize();

    // Calculate text position: center-bottom with some padding
    sf::FloatRect textBounds = hoveredCityText.getLocalBounds();
    float x = (windowSize.x - textBounds.width) / 2.0f;
    float y = windowSize.y - textBounds.height - 10.0f; // 10 pixels from bottom

    hoveredCityText.setPosition(x, y);

    // Set view to default for UI rendering
    sf::View originalView = window.getView();
    window.setView(window.getDefaultView());

    window.draw(hoveredCityText);

    // Restore original view
    window.setView(originalView);
}
