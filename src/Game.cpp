// src/Game.cpp
#include "Game.h"
#include "Utilities.h"

// Constructor
Game::Game(int chunkSize, int tileSize, int WORLD_CHUNKS_X, int WORLD_CHUNKS_Y)
    : CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    chunkManager(WORLD_CHUNKS_X, WORLD_CHUNKS_Y, CHUNK_SIZE, TILE_SIZE),
    inputHandler(view),
    renderer(window, view, chunkManager, CHUNK_SIZE, TILE_SIZE)
{
    // Calculate world size in pixels
    float worldSizeX = chunkManager.WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = chunkManager.WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    // Create a window with a specified resolution
    window.create(sf::VideoMode(800, 600), "2D Transport Game");
    window.setFramerateLimit(120);

    // Set initial view to match the window size
    view = sf::View(sf::FloatRect(0, 0, 800, 600));
    view.setCenter(worldSizeX / 2.0f, worldSizeY / 2.0f);
}

// Destructor
Game::~Game() {
    // Any necessary cleanup can be done here
}

// Main game loop
void Game::run() {
    while (window.isOpen()) {
        inputHandler.processEvents(window);
        wrapView();
        renderer.renderFrame();
    }
}

// Helper function to wrap the view's center
void Game::wrapView() {
    float worldSizeX = chunkManager.WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = chunkManager.WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    sf::Vector2f center = view.getCenter();

    // Wrap the center coordinates
    center.x = Utilities::wrapFloat(center.x, worldSizeX);
    center.y = Utilities::wrapFloat(center.y, worldSizeY);

    view.setCenter(center);
}
