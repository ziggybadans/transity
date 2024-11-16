#include "StationRenderer.h"
#include <iostream>

StationRenderer::StationRenderer() {}

StationRenderer::~StationRenderer() {
    Shutdown();
}

bool StationRenderer::Init() {
    if (!font.loadFromFile("assets/PTSans-Regular.ttf")) {
        std::cerr << "Failed to load font in StationRenderer." << std::endl;
        return false;
    }

    stationNameText.setFont(font);
    stationNameText.setCharacterSize(12);
    stationNameText.setFillColor(sf::Color::Black);

    return true;
}

void StationRenderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    worldMap = map;
}

void StationRenderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    renderStations(window, camera);
}

void StationRenderer::Shutdown() {
    // Clean up resources if any
}

void StationRenderer::renderStations(sf::RenderWindow& window, const Camera& camera) {
    const auto& stations = worldMap->GetStations();
    for (const auto& station : stations) {
        sf::CircleShape shape;
        shape.setRadius(10.0f * camera.GetZoomLevel());
        shape.setFillColor(sf::Color::White);
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(2.0f);
        shape.setOrigin(shape.getRadius(), shape.getRadius());
        shape.setPosition(station.GetPosition());
        window.draw(shape);

        // Render station name
        stationNameText.setString(station.GetName());
        stationNameText.setPosition(station.GetPosition().x + 12.0f, station.GetPosition().y - 10.0f);
        window.draw(stationNameText);
    }
}
