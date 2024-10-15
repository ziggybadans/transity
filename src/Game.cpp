// src/Game.cpp
#include "Game.h"
#include "Utilities.h"

#include <imgui.h>
#include <imgui-SFML.h>

// Constructor
Game::Game(int chunkSize, int tileSize, int WORLD_CHUNKS_X, int WORLD_CHUNKS_Y)
    : CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    chunkManager(WORLD_CHUNKS_X, WORLD_CHUNKS_Y, CHUNK_SIZE, TILE_SIZE),
    windowSizeX(1920), windowSizeY(1080),
    view(sf::FloatRect(0, 0, windowSizeX, windowSizeY)),
    inputHandler(view, sf::Vector2f(windowSizeX, windowSizeY)),
    renderer(window, view, chunkManager, CHUNK_SIZE, TILE_SIZE)
{
    // Calculate world size in pixels
    float worldSizeX = chunkManager.WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = chunkManager.WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    // Create a window with a specified resolution
    window.create(sf::VideoMode(windowSizeX, windowSizeY), "2D Transport Game");
    window.setFramerateLimit(120);

    ImGui::SFML::Init(window);

    view.setCenter(worldSizeX / 2.0f, worldSizeY / 2.0f);
}

// Destructor
Game::~Game() {
    ImGui::SFML::Shutdown();
}

// Main game loop
void Game::run() {
    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Time deltaTime = deltaClock.restart();

        inputHandler.processEvents(window);
        ImGui::SFML::Update(window, deltaTime);
        drawDebugGUI();

        if (needsRegeneration && regenClock.getElapsedTime() > regenDelay) {
            chunkManager.regenerateWorld();
            needsRegeneration = false;
        }

        wrapView();
        renderer.renderFrame();
    }
}

void Game::drawDebugGUI() {
    // Create a window for debug controls
    ImGui::Begin("Debug Controls");

    // Display current settings
    ImGui::Text("World Generation Settings:");

    // Noise Frequency Slider
    if (ImGui::SliderFloat("Noise Frequency", &chunkManager.noiseFrequency, 0.001f, 0.01f, "%.5f")) {
        chunkManager.updateSettings(chunkManager.noiseFrequency, chunkManager.noiseSeed, chunkManager.landThreshold);
        needsRegeneration = true;
    }

    // Noise Seed Input
    if (ImGui::InputInt("Noise Seed", &chunkManager.noiseSeed)) {
        chunkManager.updateSettings(chunkManager.noiseFrequency, chunkManager.noiseSeed, chunkManager.landThreshold);
        needsRegeneration = true;
    }

    // Threshold for Land/Water
    if (ImGui::SliderFloat("Land Threshold", &chunkManager.landThreshold, 0.3f, 0.7f, "%.2f")) {
        needsRegeneration = true;
    }

    // Border width
    if (ImGui::SliderFloat("Mask Border Width", &chunkManager.borderWidth, 0.0f, 50.0f, "%.2f")) {
        needsRegeneration = true;
    }

    // Attenuation factor
    if (ImGui::SliderInt("Mask Attenuation Factor", &chunkManager.attenuationFactor, 1, 10)) {
        needsRegeneration = true;
    }

    // Optionally, display current frame rate
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
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
