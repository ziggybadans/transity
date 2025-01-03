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

            window.draw(tileShape);
        }
    }

    /* Cities */
    for (City city : map.GetCities()) {
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

    /* Lines */
    for (const auto& line : map.GetLines()) {
        const std::vector<BezierSegment>& segments = line.GetBezierSegments();
        float thickness = line.GetThickness();
        sf::Color color = sf::Color::Black;
        if (line.IsSelected()) {
            color = sf::Color::Yellow;
        }
        else {
            color = line.GetColor();
        }

        for (const auto& segment : segments) {
            std::vector<sf::Vector2f> bezierPoints = ComputeCubicBezier(segment, 100);

            // Draw the bezier curve as a line strip
            sf::VertexArray curve(sf::LineStrip, bezierPoints.size());
            for (size_t i = 0; i < bezierPoints.size(); ++i) {
                curve[i].position = bezierPoints[i];
                curve[i].color = color;
            }

            // Simple approach: draw multiple lines shifted perpendicularly
            int lineCount = static_cast<int>(thickness / 2); // Number of offset lines
            float halfThickness = thickness / 2.0f;

            for (int offset = -lineCount; offset <= lineCount; ++offset) {
                if (offset == 0) continue; // Skip the main line to prevent overdrawing

                sf::VertexArray thickCurve(sf::LineStrip, bezierPoints.size());
                for (size_t i = 0; i < bezierPoints.size(); ++i) {
                    // Calculate perpendicular direction
                    sf::Vector2f direction;
                    if (i < bezierPoints.size() - 1) {
                        direction = bezierPoints[i + 1] - bezierPoints[i];
                    }
                    else if (i > 0) {
                        direction = bezierPoints[i] - bezierPoints[i - 1];
                    }
                    else {
                        direction = sf::Vector2f(1.f, 0.f); // Default direction
                    }
                    // Normalize and get perpendicular
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length != 0) {
                        direction /= length;
                    }
                    sf::Vector2f perpendicular(-direction.y, direction.x);

                    // Apply offset
                    sf::Vector2f offsetVec = perpendicular * (static_cast<float>(offset));

                    thickCurve[i].position = bezierPoints[i] + offsetVec;
                    thickCurve[i].color = color;
                }
                window.draw(thickCurve);
            }

            // Draw the main curve
            window.draw(curve);

            // Draw handles for all nodes if the line is selected
            if (line.IsSelected()) {
                float handleRadius = 8.0f;
                sf::Color defaultHandleColor = sf::Color::Green;
                sf::Color selectedHandleColor = sf::Color::Yellow;
                sf::Color hoverHandleColor = sf::Color::Blue;

                for (const auto& handle : line.GetHandles()) {
                    sf::CircleShape handleShape(handleRadius);
                    handleShape.setOrigin(handleRadius, handleRadius);
                    sf::Vector2f handlePos = line.GetPointPosition(handle.index);
                    handleShape.setPosition(handlePos);
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
    }

    /* Trains */
    for (const auto& train : map.GetTrains()) {
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

std::vector<sf::Vector2f> Renderer::ComputeCubicBezier(const BezierSegment& segment, int numPoints) const {
    std::vector<sf::Vector2f> points;
    points.reserve(numPoints + 1);

    for (int i = 0; i <= numPoints; ++i) {
        float t = static_cast<float>(i) / numPoints;
        float u = 1.0f - t;

        // Cubic Bezier formula
        sf::Vector2f point = u * u * u * segment.start +
            3 * u * u * t * segment.startControl +
            3 * u * t * t * segment.endControl +
            t * t * t * segment.end;
        points.push_back(point);
    }

    return points;
}

void Renderer::Shutdown() {
    m_isInitialized = false;
}