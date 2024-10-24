#include "Renderer.h"
#include "../graphics/Camera.h"
#include <iostream>
#include <algorithm>

Renderer::Renderer()
    : isInitialized(false), cityManager(nullptr), baseCityRadius(5.0f) {} // Initialize base radius
// You can adjust the default base radius as needed

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& /*window*/, ThreadPool& /*threadPool*/) {
    // Initialize rendering resources if needed

    // Load the font
    cityFont = std::make_shared<sf::Font>();
    if (!cityFont->loadFromFile("assets/PTSans-Regular.ttf")) { // Ensure the path is correct
        std::cerr << "Renderer: Failed to load font." << std::endl;
        return false;
    }

    isInitialized = true;
    return true;
}

void Renderer::SetWorldMap(std::shared_ptr<WorldMap> map) {
    worldMap = map;
}

void Renderer::SetCityManager(CityManager* manager) {
    cityManager = manager;
}

void Renderer::SetCityCircleShape(std::shared_ptr<sf::CircleShape> shape) {
    cityShape = shape;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    // Render WorldMap
    if (worldMap) {
        worldMap->Render(window, camera);
    }

    // Render cities
    if (cityManager && cityShape) {
        float zoomLevel = camera.GetZoomLevel();
        std::vector<City> citiesToRender = cityManager->GetCitiesToRender(zoomLevel);

        // Calculate adjusted radius based on zoom level
        // Prevent division by zero and clamp the radius
        float adjustedRadius = baseCityRadius * zoomLevel;
        adjustedRadius = std::clamp(adjustedRadius, 1.0f, 50.0f); // Adjust min and max as needed

        // Temporary sf::Text object to reuse for all cities
        sf::Text cityText;
        cityText.setFont(*cityFont);

        // Adjust character size inversely based on zoom level
        unsigned int baseTextSize = 12;
        float textScale = 2.0f * zoomLevel;
        unsigned int adjustedTextSize = static_cast<unsigned int>(baseTextSize * textScale);
        adjustedTextSize = std::clamp(adjustedTextSize, 12u, 100u); // Adjust as needed

        cityText.setCharacterSize(adjustedTextSize);
        cityText.setFillColor(sf::Color::White);
        cityText.setStyle(sf::Text::Regular);
        cityText.setOutlineColor(sf::Color::Black);
        cityText.setOutlineThickness(std::clamp(1.0f * (zoomLevel), 1.0f, 10.0f));

        for (const auto& city : citiesToRender) {
            // Draw city circle with adjusted radius
            sf::CircleShape shape = *cityShape; // Copy the shape
            shape.setRadius(adjustedRadius);
            shape.setOrigin(adjustedRadius, adjustedRadius); // Re-center based on new radius
            shape.setPosition(city.position);
            shape.setOutlineThickness(std::clamp(2.0f * (zoomLevel), 1.0f, 5.0f));
            window.draw(shape);

            // Draw city name below the circle
            cityText.setString(city.name);

            // Center the text horizontally relative to the city circle
            sf::FloatRect textBounds = cityText.getLocalBounds();
            cityText.setOrigin(textBounds.left + textBounds.width / 2.0f, 0.0f);
            cityText.setPosition(city.position.x, city.position.y + adjustedRadius + 2.0f); // 2.0f is the vertical offset

            window.draw(cityText);
        }
    }
}

void Renderer::Shutdown() {
    // Clean up rendering resources if needed
    isInitialized = false;
    worldMap.reset();
    cityManager = nullptr;
    cityShape.reset();
    cityFont.reset();
    // Reset other renderable components
}
