// Renderer.cpp
#include "Renderer.h"
#include "../core/Station.h" // Include Station class
#include "../core/Line.h"     // Include Line class
#include <iostream>
#include <algorithm>

/**
<summary>
Renderer class handles all rendering-related functionality for the game, drawing various elements such as stations, lines, and place areas.
This class is responsible for managing and optimizing the rendering process for different game components, ensuring they are displayed
correctly and efficiently.
</summary>
*/
Renderer::Renderer() {}

/**
<summary>
Destructor for Renderer. Ensures any allocated resources are properly cleaned up.
</summary>
*/
Renderer::~Renderer() {
    Shutdown();
}

/**
<summary>
Initializes the Renderer by loading fonts and preparing any necessary rendering elements.
</summary>
<param name="window">Reference to the SFML RenderWindow used for rendering.</param>
<returns>True if initialization was successful, otherwise false.</returns>
*/
bool Renderer::Init(sf::RenderWindow& /*window*/) {
    isInitialized = true;

    if (!font.loadFromFile("assets/PTSans-Regular.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
        return false;
    }

    // Initialize hoveredAreaText
    hoveredAreaText.setFont(font);
    hoveredAreaText.setCharacterSize(40); // Adjust as needed
    hoveredAreaText.setFillColor(sf::Color::Black);

    return true;
}

/**
<summary>
Sets the world map that the renderer will use to render game elements.
</summary>
<param name="map">Shared pointer to the WorldMap object to be rendered.</param>
*/
void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    worldMap = map;
}

/**
<summary>
Main render function that draws all game elements, including the world map, place areas, stations, and lines.
</summary>
<param name="window">Reference to the SFML RenderWindow where elements will be drawn.</param>
<param name="camera">Reference to the Camera object used for controlling the view.</param>
*/
void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    std::lock_guard<std::mutex> lock(renderMutex);

    // Apply camera view
    if (worldMap) {
        window.setView(camera.GetView());
    }

    // Render game elements via Renderer
    renderWorldMap(window, camera);
    renderPlaceAreas(window, camera);
    renderLines(window, camera);     // Render lines first and collect line segments
    renderStations(window, camera);  // Then render stations and names

    // Render UI elements if applicable
    renderHoveredAreaName(window);
}

/**
<summary>
Renders the world map if it exists.
</summary>
<param name="window">Reference to the SFML RenderWindow.</param>
<param name="camera">Reference to the Camera object used for controlling the view.</param>
*/
void Renderer::renderWorldMap(sf::RenderWindow& window, const Camera& camera) {
    if (worldMap) {
        worldMap->Render(window, camera);
    }
}

/**
<summary>
Renders all place areas, adjusting visibility based on the current zoom level and whether the camera's view contains the area.
Highlights areas that are being hovered over by the mouse.
</summary>
<param name="window">Reference to the SFML RenderWindow.</param>
<param name="camera">Reference to the Camera object used for controlling the view.</param>
*/
void Renderer::renderPlaceAreas(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& placeAreas = worldMap->GetPlaceAreas();

    // Get the camera's view bounds
    sf::Vector2f viewCenter = camera.GetView().getCenter();
    sf::Vector2f viewSize = camera.GetView().getSize();
    sf::FloatRect cameraRect(viewCenter.x - viewSize.x / 2.0f,
        viewCenter.y - viewSize.y / 2.0f,
        viewSize.x,
        viewSize.y);

    // Get mouse position in world coordinates for hover functionality
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos, camera.GetView());

    // Reset hoveredAreaName
    hoveredAreaName.clear();

    // Calculate zoom factor
    float currentZoom = camera.GetZoomLevel();

    // Iterate through each place area
    for (const auto& area : placeAreas) {
        // Skip rendering based on zoom level
        float zoomThreshold = 0.0f;
        switch (area.category) {
        case PlaceCategory::City:
            zoomThreshold = 1.0f;
            break;
        case PlaceCategory::Town:
            zoomThreshold = 0.1f;
            break;
        case PlaceCategory::Suburb:
            zoomThreshold = 0.01f;
            break;
        default:
            break;
        }
        if (currentZoom > zoomThreshold) {
            continue; // Skip rendering this area
        }

        // Check if the area's bounding box intersects with the camera's view
        if (!cameraRect.intersects(area.bounds)) {
            continue; // Skip rendering if not in view
        }

        // Check if mouse is over the area
        if (area.bounds.contains(mouseWorldPos)) {
            // Perform point-in-polygon test
            bool containsPoint = false;
            for (size_t i = 0; i < area.filledShape.getVertexCount(); i += 3) {
                // Create triangle
                sf::Vector2f p1 = area.filledShape[i].position;
                sf::Vector2f p2 = area.filledShape[i + 1].position;
                sf::Vector2f p3 = area.filledShape[i + 2].position;

                // Barycentric technique
                float denominator = ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));
                if (denominator == 0.0f) continue; // Prevent division by zero

                float a = ((p2.y - p3.y) * (mouseWorldPos.x - p3.x) + (p3.x - p2.x) * (mouseWorldPos.y - p3.y)) / denominator;
                float b = ((p3.y - p1.y) * (mouseWorldPos.x - p3.x) + (p1.x - p3.x) * (mouseWorldPos.y - p3.y)) / denominator;
                float c = 1.0f - a - b;

                if (a >= 0 && b >= 0 && c >= 0) {
                    containsPoint = true;
                    break;
                }
            }
            if (containsPoint) {
                hoveredAreaName = area.name;

                // Create a temporary filled shape with yellow color
                sf::VertexArray highlightedFill = area.filledShape;
                for (size_t i = 0; i < highlightedFill.getVertexCount(); ++i) {
                    highlightedFill[i].color = sf::Color::Yellow; // Highlight fill color
                }
                window.draw(highlightedFill);

                // Draw the outline as normal
                window.draw(area.outline);
                continue;
            }
        }

        // Draw the regular filled shape and outline
        window.draw(area.filledShape);
        window.draw(area.outline);
    }
}

/**
<summary>
Renders all stations, highlighting them if they are being hovered over by the mouse.
</summary>
<param name="window">Reference to the SFML RenderWindow.</param>
<param name="camera">Reference to the Camera object used for controlling the view.</param>
*/
void Renderer::renderStations(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& stations = worldMap->GetStations();

    // Get the selected station
    Station* selectedStation = worldMap->GetSelectedStation();

    // For collision detection, convert line segments to screen coordinates
    std::vector<std::pair<sf::Vector2f, sf::Vector2f>> screenLineSegments;
    for (const auto& segment : lineSegments) {
        sf::Vector2i p1i = window.mapCoordsToPixel(segment.first, camera.GetView());
        sf::Vector2i p2i = window.mapCoordsToPixel(segment.second, camera.GetView());
        sf::Vector2f p1(static_cast<float>(p1i.x), static_cast<float>(p1i.y));
        sf::Vector2f p2(static_cast<float>(p2i.x), static_cast<float>(p2i.y));
        screenLineSegments.emplace_back(p1, p2);
    }

    // For each station
    for (const auto& station : stations) {
        bool isHovered = false;
        bool isSelected = (&station == selectedStation);

        // Render the station icon in world coordinates
        station.Render(window, camera.GetZoomLevel(), isHovered, isSelected);

        // Get the station's position in world coordinates
        sf::Vector2f stationPos = station.GetPosition();

        // Convert to screen coordinates
        sf::Vector2i screenPosi = window.mapCoordsToPixel(stationPos, camera.GetView());
        sf::Vector2f screenPos(static_cast<float>(screenPosi.x), static_cast<float>(screenPosi.y));

        // Decide where to place the text relative to the station icon
        enum Position { Below, Above, Right, Left };
        std::vector<Position> positions = { Below, Above, Right, Left };

        sf::Text stationText;
        stationText.setFont(font);
        stationText.setString(station.GetName());
        stationText.setCharacterSize(15); // Fixed character size
        stationText.setFillColor(sf::Color::Black);
        stationText.setOutlineThickness(0);

        // Since we're drawing in screen coordinates, we need to use the default view
        window.setView(window.getDefaultView());

        float padding = 4.0f;

        sf::FloatRect textRect = stationText.getLocalBounds();

        sf::RectangleShape backgroundRect;
        backgroundRect.setFillColor(sf::Color::White);
        backgroundRect.setOutlineColor(sf::Color::Black);
        backgroundRect.setOutlineThickness(1.0f); // Fixed outline thickness

        bool positionFound = false;

        for (Position pos : positions) {
            sf::Vector2f textPosition;

            switch (pos) {
            case Below:
                textPosition = sf::Vector2f(screenPos.x - textRect.width / 2, screenPos.y + 15);
                break;
            case Above:
                textPosition = sf::Vector2f(screenPos.x - textRect.width / 2, screenPos.y - textRect.height - 15);
                break;
            case Right:
                textPosition = sf::Vector2f(screenPos.x + 5, screenPos.y - textRect.height / 2);
                break;
            case Left:
                textPosition = sf::Vector2f(screenPos.x - textRect.width - 5, screenPos.y - textRect.height / 2);
                break;
            }

            stationText.setPosition(textPosition);
            textRect = stationText.getGlobalBounds();

            backgroundRect.setPosition(textRect.left - padding, textRect.top - padding);
            backgroundRect.setSize(sf::Vector2f(textRect.width + 2 * padding, textRect.height + 2 * padding));

            // Check for collision with lines in screen coordinates
            bool collision = false;

            for (const auto& segment : screenLineSegments) {
                if (rectangleIntersectsLine(textRect, segment.first, segment.second)) {
                    collision = true;
                    break;
                }
            }

            if (!collision) {
                // No collision, draw text and background
                window.draw(backgroundRect);
                window.draw(stationText);

                positionFound = true;
                break;
            }
        }

        if (!positionFound) {
            // Default to below
            sf::Vector2f textPosition = sf::Vector2f(screenPos.x - textRect.width / 2, screenPos.y + 15);
            stationText.setPosition(textPosition);
            textRect = stationText.getGlobalBounds();

            backgroundRect.setPosition(textRect.left, textRect.top);
            backgroundRect.setSize(sf::Vector2f(textRect.width, textRect.height));

            window.draw(backgroundRect);
            window.draw(stationText);
        }

        // Reset view to the camera's view for subsequent drawing
        window.setView(camera.GetView());
    }
}

/**
<summary>
Renders all lines, highlighting the selected line and drawing a preview for the line currently being built.
</summary>
<param name="window">Reference to the SFML RenderWindow.</param>
<param name="camera">Reference to the Camera object used for controlling the view.</param>
*/
void Renderer::renderLines(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& lines = worldMap->GetLines();
    float currentZoom = camera.GetZoomLevel();

    Line* selectedLine = worldMap->GetSelectedLine();

    lineSegments.clear(); // Clear previous line segments

    // Render existing lines recursively
    for (const auto& linePtr : lines) {
        auto& line = *linePtr; // Dereference the unique_ptr to get the Line object

        // Render the line and its branches recursively
        renderLineRecursive(&line, window, currentZoom, selectedLine);
    }

    // Render the current line being built or extended
    const Line* currentLine = worldMap->GetCurrentLine();
    const Line* lineBeingExtended = worldMap->GetLineBeingExtended();
    bool isExtendingLine = worldMap->IsExtendingLine();
    int extendNodeIndex = worldMap->GetExtendNodeIndex();

    if (currentLine || lineBeingExtended) {
        // Decide which line to use
        const Line* lineToUse = currentLine ? currentLine : lineBeingExtended;

        // We don't need to render lineToUse here because it has already been rendered in the existing lines
        // We need to render the preview from the appropriate node to the current mouse position

        // Draw the preview line from the last node to the current mouse position
        const auto& nodes = lineToUse->GetNodes();
        if (!nodes.empty()) {
            sf::Vector2f previewStart;
            std::vector<sf::Vector2f> tempNodePositions;
            tempNodePositions.reserve(nodes.size() + 1); // Reserve space

            if (isExtendingLine) {
                // Extending an existing line
                if (extendNodeIndex == 0) {
                    // Extending from the start
                    previewStart = nodes.front().GetPosition();
                    tempNodePositions.push_back(worldMap->currentMousePosition); // New point at the beginning
                    for (const auto& node : nodes) {
                        tempNodePositions.push_back(node.GetPosition());
                    }
                }
                else {
                    // Extending from the end
                    previewStart = nodes.back().GetPosition();
                    for (const auto& node : nodes) {
                        tempNodePositions.push_back(node.GetPosition());
                    }
                    tempNodePositions.push_back(worldMap->currentMousePosition); // New point at the end
                }
            }
            else {
                // Building a new line or branch
                previewStart = nodes.back().GetPosition();
                for (const auto& node : nodes) {
                    tempNodePositions.push_back(node.GetPosition());
                }
                tempNodePositions.push_back(worldMap->currentMousePosition);
            }

            // Generate spline points
            std::vector<sf::Vector2f> tempSplinePoints;
            const int numPointsPerSegment = 20;

            for (size_t i = 0; i < tempNodePositions.size() - 1; ++i) {
                sf::Vector2f p0 = (i == 0) ? tempNodePositions[i] : tempNodePositions[i - 1];
                sf::Vector2f p1 = tempNodePositions[i];
                sf::Vector2f p2 = tempNodePositions[i + 1];
                sf::Vector2f p3 = (i + 2 < tempNodePositions.size()) ? tempNodePositions[i + 2] : tempNodePositions[i + 1];

                for (int j = 0; j <= numPointsPerSegment; ++j) {
                    float t = static_cast<float>(j) / numPointsPerSegment;
                    sf::Vector2f point = 0.5f * (
                        (2.0f * p1) +
                        (-p0 + p2) * t +
                        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
                        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t
                        );
                    tempSplinePoints.push_back(point);
                }
            }

            // Render the preview spline
            sf::VertexArray previewLine(sf::LineStrip, tempSplinePoints.size());
            for (size_t i = 0; i < tempSplinePoints.size(); ++i) {
                previewLine[i].position = tempSplinePoints[i];
                previewLine[i].color = sf::Color::Red; // Preview color
            }

            window.draw(previewLine);
        }
    }

    // Render trains
    for (const auto& linePtr : lines) {
        auto& line = *linePtr; // Dereference the unique_ptr to get the Line object

        // Render trains recursively
        std::function<void(const Line*)> renderTrainsRecursive = [&](const Line* currentLine) {
            const auto& trains = currentLine->GetTrains();
            for (const auto& train : trains) {
                train.Render(window, currentZoom);
            }

            // Recursively render trains on child lines
            for (const auto& childLine : currentLine->GetChildLines()) {
                renderTrainsRecursive(childLine.get());
            }
            };

        renderTrainsRecursive(&line);
    }
}

/**
<summary>
Renders the name of the hovered area, displaying it at the bottom of the screen.
</summary>
<param name="window">Reference to the SFML RenderWindow.</param>
*/
void Renderer::renderHoveredAreaName(sf::RenderWindow& window) {
    if (hoveredAreaName.empty()) return;

    hoveredAreaText.setString(hoveredAreaName);

    // Get window size
    sf::Vector2u windowSize = window.getSize();

    // Calculate text position: center-bottom with some padding
    sf::FloatRect textBounds = hoveredAreaText.getLocalBounds();
    float x = (windowSize.x - textBounds.width) / 2.0f;
    float y = windowSize.y - textBounds.height - 40.0f; // 40 pixels from bottom

    hoveredAreaText.setPosition(x, y);

    // Set view to default for UI rendering
    sf::View originalView = window.getView();
    window.setView(window.getDefaultView());

    window.draw(hoveredAreaText);

    // Restore original view
    window.setView(originalView);
}

/**
<summary>
Shuts down the renderer and releases any resources held by it.
</summary>
*/
void Renderer::Shutdown() {
    isInitialized = false;
    worldMap.reset();
}

bool Renderer::rectangleIntersectsLine(const sf::FloatRect& rect, const sf::Vector2f& p1, const sf::Vector2f& p2) {
    // Get the four corners of the rectangle
    sf::Vector2f topLeft(rect.left, rect.top);
    sf::Vector2f topRight(rect.left + rect.width, rect.top);
    sf::Vector2f bottomLeft(rect.left, rect.top + rect.height);
    sf::Vector2f bottomRight(rect.left + rect.width, rect.top + rect.height);

    // Check if the line segment intersects any of the rectangle's edges
    if (lineSegmentsIntersect(p1, p2, topLeft, topRight)) return true;
    if (lineSegmentsIntersect(p1, p2, topRight, bottomRight)) return true;
    if (lineSegmentsIntersect(p1, p2, bottomRight, bottomLeft)) return true;
    if (lineSegmentsIntersect(p1, p2, bottomLeft, topLeft)) return true;

    return false;
}

bool Renderer::lineSegmentsIntersect(const sf::Vector2f& p1, const sf::Vector2f& p2,
    const sf::Vector2f& q1, const sf::Vector2f& q2) {
    auto orientation = [](const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& c) {
        float val = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
        if (val == 0.0f) return 0;  // Colinear
        return (val > 0.0f) ? 1 : 2; // Clock or counterclock wise
        };

    int o1 = orientation(p1, p2, q1);
    int o2 = orientation(p1, p2, q2);
    int o3 = orientation(q1, q2, p1);
    int o4 = orientation(q1, q2, p2);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    return false; // Doesn't fall in any of the above cases
}

void Renderer::renderLineRecursive(const Line* line, sf::RenderWindow& window, float zoomLevel, const Line* selectedLine) {
    if (!line) return;

    // Determine if this line should be highlighted
    bool isSelected = false;

    if (line == selectedLine) {
        // This is the selected line
        isSelected = true;
    }
    else if (selectedLine && selectedLine->IsDescendantOf(line)) {
        // The selected line is a descendant of this line
        isSelected = true;
    }

    // Render the line
    line->Render(window, zoomLevel, isSelected);

    // Collect line segments from the line
    const std::vector<sf::Vector2f>& splinePoints = line->GetSplinePoints();
    for (size_t i = 1; i < splinePoints.size(); ++i) {
        lineSegments.emplace_back(splinePoints[i - 1], splinePoints[i]);
    }

    if (line->IsEditing()) {
        // Existing code for rendering editing nodes...
    }

    // Recursively render child lines
    for (const auto& childLine : line->GetChildLines()) {
        renderLineRecursive(childLine.get(), window, zoomLevel, selectedLine);
    }
}