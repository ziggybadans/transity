#include "Renderer.h"
#include "../graphics/Camera.h"
#include <iostream>

Renderer::Renderer()
    : isInitialized(false), cityManager(nullptr) {}

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

        // Temporary sf::Text object to reuse for all cities
        sf::Text cityText;
        cityText.setFont(*cityFont);
        // Adjust character size based on zoom level
        // Example: Character size increases as zoom level increases
        unsigned int baseSize = 12;
        unsigned int adjustedSize = static_cast<unsigned int>(baseSize * zoomLevel);
        adjustedSize = std::max(adjustedSize, 8u); // Minimum size
        cityText.setCharacterSize(adjustedSize);
        cityText.setFillColor(sf::Color::White);
        cityText.setStyle(sf::Text::Regular);

        for (const auto& city : citiesToRender) {
            // Draw city circle
            sf::CircleShape shape = *cityShape; // Copy the shape
            shape.setPosition(city.position);
            window.draw(shape);

            // Draw city name below the circle
            cityText.setString(city.name);
            // Center the text horizontally relative to the city circle
            sf::FloatRect textBounds = cityText.getLocalBounds();
            cityText.setOrigin(textBounds.left + textBounds.width / 2.0f, 0.0f);
            cityText.setPosition(city.position.x, city.position.y + cityShape->getRadius() + 2.0f); // 2.0f is the offset below the circle

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
