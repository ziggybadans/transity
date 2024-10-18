// src/Renderer.h
#pragma once

#include <SFML/Graphics.hpp>
#include "ChunkManager.h"
#include "Chunk.h"
#include <unordered_map>
#include <memory>
#include <cmath>
#include <array> // Include array for std::array

// Renderer class responsible for rendering chunks with LOD
class Renderer {
public:
    // Constructor
    Renderer(sf::RenderWindow& window, sf::View& view, ChunkManager& chunkManager,
        int chunkSize, int tileSize, const sf::Vector2f& defaultSize);

    // Renders a single frame
    void renderFrame();

private:
    // Reference to the SFML window
    sf::RenderWindow& m_window;

    // Reference to the current view
    sf::View& m_view;

    // Reference to the chunk manager
    ChunkManager& m_chunkManager; // Changed to non-const to allow dynamic loading

    // Size of a chunk in number of tiles
    const int m_chunkSize;

    // Size of a tile in pixels
    const int m_tileSize;

    // Default view size for LOD calculations
    sf::Vector2f m_defaultViewSize;

    // LOD thresholds (Add more thresholds as needed)
    static constexpr float LOD_THRESHOLDS[] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
    static constexpr size_t NUM_LODS = sizeof(LOD_THRESHOLDS) / sizeof(float);

    // Cached LOD level and last zoom factor
    int m_cachedLOD = -1;
    float m_lastZoom = 0.0f;

    // VertexArrays for different LODs
    std::array<sf::VertexArray, 5> m_vaLODs; // Supports LOD0 to LOD4

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

    // Helper to calculate visible chunk range
    void loadVisibleChunks(const sf::FloatRect& viewBounds);
};
