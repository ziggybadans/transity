#include "Renderer.h"
#include <iostream>
#include "../Debug.h"
#include "../Constants.h"

Renderer::Renderer() 
    : m_isInitialized(false)
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init() {
    // Basic initialization that doesn't require a window
    m_isInitialized = false;
    return true;
}

bool Renderer::InitWithWindow(sf::RenderWindow& window) {
    if (m_isInitialized) return true;

    // Load the font
    if (!m_font.loadFromFile("data/PTSans-Regular.ttf")) { // Ensure the path is correct
        DEBUG_ERROR("Failed to load font.");
        return false;
    }

    m_isInitialized = true;
    DEBUG_INFO("Renderer initialized successfully.");
    return true;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera, const Map& map) {
    if (!m_isInitialized) return;

    RenderMap(window, map);

    std::lock_guard<std::mutex> lock(m_renderMutex);
    camera.ApplyView(window);
}

void Renderer::RenderMap(sf::RenderWindow& window, const Map& map) {
    /* Map */
    sf::RectangleShape tileShape(sf::Vector2f(Constants::TILE_SIZE, Constants::TILE_SIZE));
    for (size_t y = 0; y < map.GetSize(); ++y) {
        for (size_t x = 0; x < map.GetSize(); ++x) {
            tileShape.setPosition(x * Constants::TILE_SIZE, y * Constants::TILE_SIZE);

            if (map.GetTile(x, y) == 1) {
                tileShape.setFillColor(sf::Color::White);
            }

            window.draw(tileShape);
        }
    }

    /* Lines */
    for (auto& line : map.m_lines) {
        std::vector<City*> cities = line.GetCities();
        float thickness = line.thickness;
        sf::Color color = line.color;

        for (size_t i = 0; i < cities.size() - 1; ++i) {
            sf::Vector2f start = cities[i]->position;
            sf::Vector2 end = cities[i + 1]->position;

            sf::Vector2f direction = end - start;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159265f;

            sf::RectangleShape lineShape(sf::Vector2f(length, thickness));
            lineShape.setPosition(start);
            lineShape.setFillColor(color);
            lineShape.setRotation(angle);
            lineShape.setOrigin(0, thickness / 2);
            window.draw(lineShape);
        }
    }

    /* Cities */
    for (City city : map.m_cities) {
        sf::CircleShape circleShape(city.radius);
        circleShape.setOrigin(city.radius, city.radius);
        circleShape.setPosition(city.position);
        circleShape.setFillColor(sf::Color::Black);
        window.draw(circleShape);

        // Create and configure the text label
        sf::Text text;
        text.setFont(m_font); // Use the loaded font
        text.setString(city.name);
        text.setCharacterSize(14); // Adjust as needed
        text.setFillColor(sf::Color::Black); // Choose a color that contrasts with the map

        // Calculate the position: below the city
        sf::FloatRect textBounds = text.getLocalBounds();
        float textX = city.position.x - textBounds.width / 2;
        float textY = city.position.y + city.radius + 5.0f; // 5 pixels below the city

        text.setPosition(textX, textY);
        window.draw(text);
    }

    // Render all trains
    for (const auto& train : map.m_trains) {
        sf::CircleShape trainShape(6.f); // small circle for the train
        trainShape.setOrigin(6.f, 6.f);
        if (train.IsSelected()) {
            trainShape.setFillColor(sf::Color::Yellow);
        }
        else {
            trainShape.setFillColor(sf::Color::Red);
        }
        trainShape.setPosition(train.GetPosition());
        window.draw(trainShape);
    }
}

void Renderer::Shutdown() {
    m_isInitialized = false;
}