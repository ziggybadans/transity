// src/Renderer.h
#pragma once

#include <SFML/Graphics.hpp>
#include "ChunkManager.h"

class Renderer {
public:
    Renderer(sf::RenderWindow& window, sf::View& view, const ChunkManager& chunkManager, int chunkSize, int tileSize, const sf::Vector2f& defaultSize);

    void renderFrame();

private:
    sf::RenderWindow& window;
    sf::View& view;
    const ChunkManager& chunkManager;
    const int m_chunkSize;
    const int m_tileSize;

    std::vector<sf::Transform> m_gridTransforms;

    static constexpr float LOD1_THRESHOLD = 1.0f;
    static constexpr float LOD2_THRESHOLD = 3.0f;

    sf::Vector2f defaultViewSize;

    int m_cachedLOD = -1;
    float m_lastZoom = 0.0f;

    int determineLODLevel();

    void drawChunks();
};