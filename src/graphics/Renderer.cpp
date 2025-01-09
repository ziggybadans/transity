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
    m_font.setSmooth(true);

    m_isInitialized = true;
    DEBUG_INFO("Renderer initialized successfully.");
    return true;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera, Map& map, StateManager& stateManager) {
    if (!m_isInitialized) return;

    RenderMap(window, map, camera, stateManager);

    std::lock_guard<std::mutex> lock(m_renderMutex);
    camera.ApplyView(window);
}

void Renderer::RenderMap(sf::RenderWindow& window, Map& map, const Camera& camera, StateManager& stateManager) const {
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

    City* startCity = map.GetStartCityForTrain();
    City* endCity = map.GetEndCityForTrain();

    /* Lines */
    for (const auto& line : map.GetLines()) {
        // Determine if the line is selected via SelectionManager
        bool isSelected = (&line) == map.GetSelectedLine();

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
    }

    // Route
    if (stateManager.GetState<std::string>("CurrentTool") == "TrainPlace"
        && startCity && endCity) {
        // Use the new route-finding with nodes from the map
        std::vector<Node*> routeNodes = map.FindRouteBetweenNodes(startCity, endCity);
        if (!routeNodes.empty()) {
            // Draw the route segments in orange
            sf::Color highlightColor = sf::Color(255, 165, 0); // Orange
            float highlightThickness = 6.0f; // Thicker than normal lines
            for (size_t i = 0; i + 1 < routeNodes.size(); ++i) {
                // For each adjacent pair of nodes, find the connecting line segment
                Node* nodeA = routeNodes[i];
                Node* nodeB = routeNodes[i + 1];
                // Find the adjusted points for visualization
                for (auto& line : map.GetLines()) {
                    const auto& points = line.GetPoints();
                    for (size_t j = 0; j + 1 < points.size(); ++j) {
                        if ((points[j].node == nodeA && points[j + 1].node == nodeB) ||
                            (points[j].node == nodeB && points[j + 1].node == nodeA)) {
                            auto adjustedPoints = line.GetAdjustedPathPoints();
                            if (j < adjustedPoints.size() && (j + 1) < adjustedPoints.size()) {
                                sf::Vector2f start = adjustedPoints[j];
                                sf::Vector2f end = adjustedPoints[j + 1];
                                DrawThickLine(window, start, end, highlightThickness, highlightColor);
                            }
                        }
                    }
                }
            }
        }
    }

    /* Cities */
    for (const City& city : map.GetCities()) {
        sf::CircleShape circleShape(city.GetRadius());
        circleShape.setOrigin(city.GetRadius(), city.GetRadius());
        circleShape.setPosition(city.GetPosition());
        circleShape.setFillColor(city.IsSelected() ? sf::Color::Yellow : sf::Color::Black);
        circleShape.setOutlineColor(sf::Color::Black);
        circleShape.setOutlineThickness(2.0f);
        window.draw(circleShape);

        // Create and configure the text label
        sf::Text text;
        text.setFont(m_font); // Use the loaded font
        text.setString(city.GetName());
        text.setFillColor(sf::Color::Black); // Choose a color that contrasts with the map
        text.setCharacterSize(32.0f / camera.GetZoomLevel()); // Adjust as needed
        text.setScale(sf::Vector2f(camera.GetZoomLevel() / 2, camera.GetZoomLevel() / 2));

        // Calculate the position: below the city
        sf::FloatRect textBounds = text.getLocalBounds();
        float scaledWidth = textBounds.width * text.getScale().x;
        float scaledHeight = textBounds.height * text.getScale().y;
        float textX = city.GetPosition().x - scaledWidth / 2;
        float textY = city.GetPosition().y + city.GetRadius() + 5.0f; // 5 pixels below the city

        text.setPosition(textX, textY);
        window.draw(text);

    }

    for (const Node& node : map.GetNodes()) {
        sf::CircleShape circleShape(node.GetRadius());
        circleShape.setOrigin(node.GetRadius(), node.GetRadius());
        circleShape.setPosition(node.GetPosition());
        circleShape.setFillColor(node.IsSelected() ? sf::Color::Yellow : sf::Color::Black);
        window.draw(circleShape);
    }

    /* Line Handles */
    for (const auto& line : map.GetLines()) {
        // Determine if the line is selected via SelectionManager
        bool isSelected = (&line) == map.GetSelectedLine();

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

    /* Train Stop Indicators */
    for (const City& city : map.GetCities()) {
        sf::Text indicatorText;
        indicatorText.setFont(m_font);
        indicatorText.setCharacterSize(72); // Larger size for visibility
        indicatorText.setFillColor(sf::Color::White); // Choose a distinct color
        indicatorText.setCharacterSize(32.0f / camera.GetZoomLevel()); // Adjust as needed
        indicatorText.setScale(sf::Vector2f(camera.GetZoomLevel() / 2, camera.GetZoomLevel() / 2));

        if (&city == startCity) {
            indicatorText.setString("1");
        }
        else if (&city == endCity) {
            indicatorText.setString("2");
        }
        else {
            continue; // No indicator needed
        }

        // Position the indicator at the top-right corner of the city
        sf::FloatRect indicatorBounds = indicatorText.getLocalBounds();
        float scaledWidth = indicatorBounds.width * indicatorText.getScale().x;
        float scaledHeight = indicatorBounds.height * indicatorText.getScale().y;
        float indicatorX = city.GetPosition().x + city.GetRadius() - 11.0f - scaledWidth / 2;
        float indicatorY = city.GetPosition().y - city.GetRadius() + 5.0f - scaledHeight / 2;

        indicatorText.setPosition(indicatorX, indicatorY);
        window.draw(indicatorText);
    }

    /* Trains */
    for (const auto& trainPtr : map.GetTrains()) {
        Train* train = trainPtr.get();
        bool isSelected = (train) == map.GetSelectedTrain();

        sf::Vector2f position = train->GetPosition();

        // Compute direction for orientation
        sf::Vector2f nextPoint = position;
        const auto& pathPoints = train->GetPathPoints();
        int nextIdx = train->GetCurrentPointIndex() + (train->GetDirection() == "Forward" ? 1 : -1);
        if (!pathPoints.empty() && nextIdx >= 0 && nextIdx < static_cast<int>(pathPoints.size())) {
            nextPoint = pathPoints[nextIdx];
        }
        sf::Vector2f dir = nextPoint - position;
        float angle = std::atan2(dir.y, dir.x) * 180.0f / 3.14159265f;

        sf::ConvexShape trainShape;
        trainShape.setPointCount(5);
        float length = 30.f;
        float width = 10.f;
        trainShape.setPoint(0, sf::Vector2f(-length / 2, -width / 2));
        trainShape.setPoint(1, sf::Vector2f(-length / 2, width / 2));
        trainShape.setPoint(2, sf::Vector2f(length / 4, width / 2));
        trainShape.setPoint(3, sf::Vector2f(length / 2, 0.f));
        trainShape.setPoint(4, sf::Vector2f(length / 4, -width / 2));

        trainShape.setFillColor(isSelected ? sf::Color::Yellow : sf::Color::Red);
        sf::FloatRect bounds = trainShape.getLocalBounds();
        trainShape.setOrigin(bounds.width / 2, bounds.height / 2);
        trainShape.setPosition(position);
        trainShape.setRotation(angle);
        window.draw(trainShape);

        // Draw passenger count inside the train shape
        sf::Text countText;
        countText.setFont(m_font);
        countText.setString(std::to_string(train->GetPassengerCount()));
        countText.setFillColor(sf::Color::Black);
        countText.setCharacterSize(14);
        sf::FloatRect textBounds = countText.getLocalBounds();
        countText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        countText.setPosition(position);
        window.draw(countText);
    }

    for (const City& city : map.GetCities()) {
        const auto& waiting = city.GetWaitingPassengers();
        for (size_t i = 0; i < waiting.size(); ++i) {
            sf::RectangleShape passengerShape(sf::Vector2f(5.f, 5.f));
            passengerShape.setFillColor(sf::Color::Blue);
            float offsetX = (i % 5) * 6.0f - 15.f;
            float offsetY = (i / 5) * 6.0f - 15.f;
            passengerShape.setPosition(city.GetPosition().x + offsetX,
                city.GetPosition().y + offsetY);
            window.draw(passengerShape);
        }
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