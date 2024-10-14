// src/Game.h
#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Chunk.h"

class Game {
public:
    /**
     * @brief Constructor for the Game class.
     * Initializes the game window, sets up the initial view, and starts the background chunk generation thread.
     * @param chunkSize The number of tiles per chunk.
     * @param tileSize The size of each tile in pixels.
     */
    Game(int chunkSize, int tileSize);

    /**
     * @brief Destructor for the Game class.
     * Signals the background chunk generation thread to stop and waits for it to finish.
     */
    ~Game();

    /**
     * @brief Runs the main game loop.
     * Continuously processes events, updates the game state, and renders the scene until the window is closed.
     */
    void run();

private:
    sf::RenderWindow window;
    sf::View view;

    // Game constants
    static constexpr int WORLD_CHUNKS_X = 10; // Number of chunks in the X direction
    static constexpr int WORLD_CHUNKS_Y = 10; // Number of chunks in the Y direction
    const int CHUNK_SIZE; // Size of each chunk (number of tiles per chunk)
    const int TILE_SIZE;  // Size of each tile in pixels

    // A map to keep track of generated chunks
    std::unordered_map<ChunkCoord, Chunk> chunks;

    // Synchronization mutex for chunks map
    std::mutex chunksMutex;

    /**
     * @brief Processes user input events.
     * Handles window close events and manages camera movement based on keyboard input.
     */
    void processEvents();

    int wrapCoordinate(int coord, int max);
    float wrapFloat(float coord, float max);
    void wrapView();

    int calculateSteps(int start, int end, int max);

    /**
     * @brief Generates a chunk of tiles.
     * Creates a chunk at the given coordinates with alternating land and water tiles.
     * @param chunkX The x-coordinate of the chunk.
     * @param chunkY The y-coordinate of the chunk.
     * @return A generated chunk with tiles.
     */
    Chunk generateChunk(int chunkX, int chunkY);

    /**
     * @brief Renders the current state of the game.
     * Clears the window, sets the view, and draws all visible chunks and tiles.
     */
    void render();
};