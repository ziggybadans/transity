// src/Renderer.h
#pragma once

#include <SFML/Graphics.hpp>
#include "ChunkManager.h"

class Renderer {
public:
    Renderer(sf::RenderWindow& window, sf::View& view, const ChunkManager& chunkManager, int chunkSize, int tileSize);

    // Render the current frame
    void renderFrame();

private:
    sf::RenderWindow& window;
    sf::View& view;
    const ChunkManager& chunkManager;
    int CHUNK_SIZE;
    int TILE_SIZE;

    void drawChunks();
};