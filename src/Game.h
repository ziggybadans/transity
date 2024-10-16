// src/Game.h
#pragma once

#include <SFML/Graphics.hpp>
#include "ChunkManager.h"
#include "InputHandler.h"
#include "Renderer.h"
#include "HeightMap.h"
#include <unordered_set>
#include <future>

class Game {
public:
    Game(int chunkSize, int tileSize, int WORLD_CHUNKS_X, int WORLD_CHUNKS_Y, const std::string& heightMapPath = "");
    ~Game();

    // Run the main game loop
    void run();

private:
    int windowSizeX;
    int windowSizeY;

    // Window and view
    sf::RenderWindow window;
    sf::View view;

    // Game settings
    const int CHUNK_SIZE;
    const int TILE_SIZE;

    // Managers and handlers
    ChunkManager chunkManager;
    InputHandler inputHandler;
    Renderer renderer;
    HeightMap* heightMap;

    // Helper function to wrap the view's center
    void wrapView();
    void drawDebugGUI();

    // Dynamic chunk management
    std::unordered_set<ChunkCoord, std::hash<ChunkCoord>> activeChunks;
    std::vector<std::future<Chunk>> loadingChunks;
    std::mutex activeChunksMutex;

    // View parameters
    float renderDistance; // Number of chunks to load around the view

    // Chunk loading parameters
    void updateVisibleChunks();
    ChunkCoord getChunkCoordFromPosition(const sf::Vector2f& position) const;

    bool needsRegeneration = false;
    sf::Clock regenClock;
    sf::Time regenDelay = sf::seconds(0.5f); // 0.5-second delay
};