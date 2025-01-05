// Renderer.cpp
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

void Renderer::Render(sf::RenderWindow& window, const Camera& camera, Map& map) {
    if (!m_isInitialized) return;

    RenderMap(window, map, camera);

    std::lock_guard<std::mutex> lock(m_renderMutex);
    camera.ApplyView(window);
}

void Renderer::RenderMap(sf::RenderWindow& window, Map& map, const Camera& camera) const {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera.GetView());
    const float BOUNDING_BOX_RADIUS = 10.0f;
    float boundingBoxSquared = BOUNDING_BOX_RADIUS * BOUNDING_BOX_RADIUS;

    /* Map */
    sf::RectangleShape tileShape(sf::Vector2f(Constants::TILE_SIZE, Constants::TILE_SIZE));
    for (size_t y = 0; y < map.GetSize(); ++y) {
        for (size_t x = 0; x < map.GetSize(); ++x) {
            tileShape.setPosition(x * Constants::TILE_SIZE, y * Constants::TILE_SIZE);

            if (map.GetTile(x, y) == 1) {
                tileShape.setFillColor(sf::Color::White);
            }
            else {
                tileShape.setFillColor(sf::Color::Transparent); // Assuming 0 is empty
            }

            window.draw(tileShape);
        }
    }

    /* Cities */
    for (const City& city : map.GetCities()) {
        sf::CircleShape circleShape(city.GetRadius());
        circleShape.setOrigin(city.GetRadius(), city.GetRadius());
        circleShape.setPosition(city.GetPosition());
        circleShape.setFillColor(sf::Color::Black);
        window.draw(circleShape);

        // Create and configure the text label
        sf::Text text;
        text.setFont(m_font); // Use the loaded font
        text.setString(city.GetName());
        text.setCharacterSize(14); // Adjust as needed
        text.setFillColor(sf::Color::Black); // Choose a color that contrasts with the map

        // Calculate the position: below the city
        sf::FloatRect textBounds = text.getLocalBounds();
        float textX = city.GetPosition().x - textBounds.width / 2;
        float textY = city.GetPosition().y + city.GetRadius() + 5.0f; // 5 pixels below the city

        text.setPosition(textX, textY);
        window.draw(text);
    }

    /* Lines */
    for (const auto& line : map.GetLines()) {
        // Determine if the line is selected via SelectionManager
        bool isSelected = (&line) == map.GetSelectionManager().GetSelectedLine();

        float thickness = line.GetThickness();
        sf::Color color = isSelected ? sf::Color::Yellow : line.GetColor();

        // Retrieve the adjusted path points for the line
        std::vector<sf::Vector2f> pathPoints = line.GetAdjustedPathPoints();

        // Iterate through consecutive point pairs to draw thick lines
        for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
            sf::Vector2f start = pathPoints[i];
            sf::Vector2f end = pathPoints[i + 1];

            // Draw thick line using the helper function
            DrawThickLine(window, start, end, thickness, color);
        }

        // Draw handles for all nodes if the line is selected
        if (isSelected) {
            float handleRadius = 8.0f;
            sf::Color defaultHandleColor = sf::Color::Green;
            sf::Color selectedHandleColor = sf::Color::Yellow;
            sf::Color hoverHandleColor = sf::Color::Blue;

            for (const auto& handle : line.GetHandles()) {
                sf::CircleShape handleShape(handleRadius);
                handleShape.setOrigin(handleRadius, handleRadius);
                sf::Vector2f handlePos = line.GetPointPosition(handle.index);
                handleShape.setPosition(handlePos);

                // Determine if the handle is hovered
                sf::Vector2f diff = handlePos - worldPos;
                float distanceSquared = diff.x * diff.x + diff.y * diff.y;
                if (distanceSquared < boundingBoxSquared) {
                    handleShape.setFillColor(hoverHandleColor);
                }
                else {
                    handleShape.setFillColor(handle.isSelected ? selectedHandleColor : defaultHandleColor);
                }
                window.draw(handleShape);
            }
        }
    }

    /* Trains */
    for (const auto& train : map.GetTrains()) {
        // Determine if the train is selected via SelectionManager
        bool isSelected = (&(*train)) == map.GetSelectionManager().GetSelectedTrain();

        sf::CircleShape trainShape(6.f); // small circle for the train
        trainShape.setOrigin(6.f, 6.f);
        if (isSelected) {
            trainShape.setFillColor(sf::Color::Yellow);
        }
        else {
            trainShape.setFillColor(sf::Color::Red);
        }
        trainShape.setPosition(train->GetPosition());
        window.draw(trainShape);
    }
}

void Renderer::Shutdown() {
    m_isInitialized = false;
}

void Renderer::DrawThickLine(sf::RenderWindow& window, const sf::Vector2f& start, const sf::Vector2f& end, float thickness, const sf::Color& color) const {
    sf::Vector2f direction = end - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length == 0) return;

    // Calculate the angle of the line in degrees
    float angle = std::atan2(direction.y, direction.x) * 180.0f / 3.14159265f;

    // Create a rectangle to represent the thick line
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(length, thickness));
    rectangle.setFillColor(color);
    rectangle.setOrigin(0, thickness / 2); // Center vertically
    rectangle.setPosition(start);
    rectangle.setRotation(angle);
    window.draw(rectangle);
}