// src/Game.h
#pragma once

#include <SFML/Graphics.hpp>
#include "ChunkManager.h"
#include "InputHandler.h"
#include "Renderer.h"

class Game {
public:
    Game(int chunkSize, int tileSize, int WORLD_CHUNKS_X, int WORLD_CHUNKS_Y);
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

    // Helper function to wrap the view's center
    void wrapView();
    void drawDebugGUI();

    bool needsRegeneration = false;
    sf::Clock regenClock;
    sf::Time regenDelay = sf::seconds(0.5f); // 0.5-second delay
};