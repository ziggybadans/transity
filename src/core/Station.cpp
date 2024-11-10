#include "Station.h"

/**
<summary>
Station class represents a station in the game world, which can be rendered and interacted with. It handles
the visual representation of a station, including its size, color, and hover effects.
</summary>
*/
Station::Station(const sf::Vector2f& position, float baseRadius, float baseOutlineThickness)
    : position(position),
    baseRadius(baseRadius),
    baseOutlineThickness(baseOutlineThickness),
    name("Unnamed Station") // Default name
{
    shape.setRadius(baseRadius); // Base radius
    shape.setFillColor(sf::Color::White); // White fill
    shape.setOutlineColor(sf::Color::Black); // Black outline
    shape.setOutlineThickness(baseOutlineThickness); // Base outline thickness
    shape.setOrigin(shape.getRadius(), shape.getRadius()); // Center the origin
    shape.setPosition(position);
}

/**
<summary>
Renders the station on the screen at its current position, adjusting its appearance based on zoom level and hover state.
</summary>
<param name="window">Reference to the SFML RenderWindow where the station will be drawn.</param>
<param name="zoomLevel">Current zoom level of the camera, used to adjust the size of the station for better visibility.</param>
<param name="isHovered">Boolean indicating whether the station is currently being hovered over.</param>
*/
void Station::Render(sf::RenderWindow& window, float zoomLevel, bool isHovered, bool isSelected) const {
    // Scale radius and outline thickness based on zoom level
    float scaledRadius = baseRadius * zoomLevel;
    float scaledOutlineThickness = baseOutlineThickness * zoomLevel;

    // Update shape's properties
    shape.setRadius(scaledRadius);
    shape.setOutlineThickness(scaledOutlineThickness);
    shape.setOrigin(scaledRadius, scaledRadius); // Re-center origin based on new radius

    // Update fill color based on hover and selection state
    if (isSelected) {
        shape.setFillColor(sf::Color::Green); // Selected station color
    }
    else if (isHovered) {
        shape.setFillColor(sf::Color::Blue); // Hovered station color
    }
    else {
        shape.setFillColor(sf::Color::White); // Default station color
    }

    // Update shape's position
    shape.setPosition(position);

    window.draw(shape);
}

/**
<summary>
Gets the current position of the station in the world.
</summary>
<returns>The current position of the station as a vector.</returns>
*/
sf::Vector2f Station::GetPosition() const {
    return position;
}

/**
<summary>
**New Method**: Sets the position of the station in the world and updates the shape's position.
</summary>
<param name="newPosition">The new position of the station as a vector.</param>
*/
void Station::SetPosition(const sf::Vector2f& newPosition) {
    position = newPosition;
    shape.setPosition(newPosition);
}

/**
<summary>
Gets the current name of the station.
</summary>
<returns>The current name of the station as a string.</returns>
*/
const std::string& Station::GetName() const {
    return name;
}

/**
<summary>
Sets the name of the station.
</summary>
<param name="name">The new name of the station.</param>
*/
void Station::SetName(const std::string& name) {
    this->name = name;
}
