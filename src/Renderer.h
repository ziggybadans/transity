// src/Renderer.h
#pragma once

#include <SFML/Graphics.hpp>
#include "ChunkManager.h"
#include "Chunk.h"
#include <unordered_map>
#include <memory>

// Renderer class responsible for rendering chunks with LOD
class Renderer {
public:
    // Constructor
    Renderer(sf::RenderWindow& window, sf::View& view, const ChunkManager& chunkManager,
        int chunkSize, int tileSize, const sf::Vector2f& defaultSize);

    // Renders a single frame
    void renderFrame();

private:
    // Reference to the SFML window
    sf::RenderWindow& m_window;

    // Reference to the current view
    sf::View& m_view;

    // Reference to the chunk manager
    const ChunkManager& m_chunkManager;

    // Size of a chunk in number of tiles
    const int m_chunkSize;

    // Size of a tile in pixels
    const int m_tileSize;

    // Default view size for LOD calculations
    sf::Vector2f m_defaultViewSize;

    // LOD thresholds
    static constexpr float LOD1_THRESHOLD = 5.0f;
    static constexpr float LOD2_THRESHOLD = 10.0f;

    // Cached LOD level and last zoom factor
    int m_cachedLOD = -1;
    float m_lastZoom = 0.0f;

    // VertexArrays for different LODs
    sf::VertexArray m_vaLOD0;
    sf::VertexArray m_vaLOD1;
    sf::VertexArray m_vaLOD2;

    // Mapping from ChunkCoord to their corresponding Chunk
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>, std::hash<ChunkCoord>> m_visibleChunks;

    // Determines the current LOD level based on view scale
    int determineLODLevel();

    // Draws the chunks based on the current LOD
    void drawChunks(int currentLOD);

    // Checks if a chunk is visible within the current view bounds
    bool isChunkVisible(const ChunkCoord& coord, const sf::FloatRect& viewBounds) const;

    // Updates the list of visible chunks based on the current view bounds
    void updateVisibleChunks(const sf::FloatRect& viewBounds);

    // Updates the VertexArrays with vertices from visible chunks
    void updateVertexArrays(int currentLOD);
};
