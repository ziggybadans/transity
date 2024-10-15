// src/Renderer.h
#pragma once

#include <SFML/Graphics.hpp>
#include "ChunkManager.h"

class Renderer {
public:
    Renderer(sf::RenderWindow& window, sf::View& view, const ChunkManager& chunkManager, int chunkSize, int tileSize, const sf::Vector2f& defaultSize);

    // Render the current frame
    void renderFrame();

private:
    sf::RenderWindow& window;
    sf::View& view;
    const ChunkManager& chunkManager;
    int CHUNK_SIZE;
    int TILE_SIZE;

    // Define zoom thresholds
    float LOD1_THRESHOLD = 5.0f; // Example value
    float LOD2_THRESHOLD = 10.0f; // Example value

    // Store the default view size
    sf::Vector2f defaultViewSize;

    mutable int cachedLOD = -1;
    mutable float lastZoom = 0.0f;

    void drawChunks();
    int determineLODLevel() const;
};