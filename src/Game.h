// src/Game.h
#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_set>
#include <future>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

#include "ChunkManager.h"
#include "InputHandler.h"
#include "Renderer.h"
#include "HeightMap.h"
#include "ThreadPool.h"

class Game {
public:
    Game(int chunkSize, int tileSize, int worldChunksX, int worldChunksY, const std::string& heightMapPath = "");
    ~Game();

    // Run the main game loop
    void run();

private:
    // Window and view
    sf::RenderWindow window;
    sf::View view;

    // Game settings
    const int CHUNK_SIZE;
    const int TILE_SIZE;
    const int WORLD_CHUNKS_X;
    const int WORLD_CHUNKS_Y;
    int windowSizeX;
    int windowSizeY;

    // Managers and handlers
    ChunkManager chunkManager;
    InputHandler inputHandler;
    Renderer renderer;
    std::shared_ptr<HeightMap> heightMap;

    // Thread pool for chunk loading
    ThreadPool threadPool;

    // Helper function to wrap the view's center
    void wrapView();
    void drawDebugGUI();

    // Dynamic chunk management
    std::unordered_set<ChunkCoord> activeChunks;
    std::vector<std::future<std::shared_ptr<Chunk>>> loadingChunks;
    std::mutex activeChunksMutex;

    // View parameters
    int renderDistance; // Number of chunks to load around the view

    // Chunk loading parameters
    void updateVisibleChunks();
    ChunkCoord getChunkCoordFromPosition(const sf::Vector2f& position) const;

    // Regeneration parameters
    bool needsRegeneration = false;
    sf::Clock regenClock;
    sf::Time regenDelay = sf::seconds(0.5f); // 0.5-second delay

    // Configuration options
    void loadHeightMap(const std::string& heightMapPath);
    void initializeWindow();

    // GUI state
    bool useRealHeightMap = false;
};
