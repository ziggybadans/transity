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

// Modified drawDebugGUI function
void Game::drawDebugGUI() {
    // Create a window for debug controls
    ImGui::Begin("Debug Controls");

    // Display current settings
    ImGui::Text("World Generation Settings:");

    // Iterate through each noise layer for dynamic controls
    for (size_t i = 0; i < chunkManager.noiseLayers.size(); ++i) {
        NoiseLayer& layer = chunkManager.noiseLayers[i];
        std::string layerLabel = "Noise Layer " + std::to_string(i + 1);
        if (ImGui::CollapsingHeader(layerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            // Noise Type Combo Box
            const char* noiseTypes[] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
            int currentNoiseType = static_cast<int>(layer.noiseType);
            if (ImGui::Combo(("Type##" + std::to_string(i)).c_str(), &currentNoiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes))) {
                layer.noiseType = static_cast<FastNoiseLite::NoiseType>(currentNoiseType);
                layer.noise.SetNoiseType(layer.noiseType);
                layer.configureNoise();
                needsRegeneration = true;
            }

            // Frequency Slider
            if (ImGui::SliderFloat(("Frequency##" + std::to_string(i)).c_str(), &layer.frequency, 0.001f, 0.1f, "%.5f")) {
                layer.noise.SetFrequency(layer.frequency);
                needsRegeneration = true;
            }

            // Amplitude Slider
            if (ImGui::SliderFloat(("Amplitude##" + std::to_string(i)).c_str(), &layer.amplitude, 0.1f, 5.0f, "%.2f")) {
                needsRegeneration = true;
            }

            // Seed Input
            if (ImGui::InputInt(("Seed##" + std::to_string(i)).c_str(), &layer.seed)) {
                layer.noise.SetSeed(layer.seed);
                needsRegeneration = true;
            }

            // Additional parameters based on noise type
            if (layer.noiseType == FastNoiseLite::NoiseType_Cellular) {
                // Cellular Distance Function
                const char* cellularDistanceFunctions[] = { "Euclidean", "EuclideanSq", "Manhattan", "Hybrid" };
                int currentDistanceFunction = static_cast<int>(layer.cellularDistanceFunction);
                if (ImGui::Combo(("Cellular Distance Function##" + std::to_string(i)).c_str(), &currentDistanceFunction, cellularDistanceFunctions, IM_ARRAYSIZE(cellularDistanceFunctions))) {
                    layer.cellularDistanceFunction = static_cast<FastNoiseLite::CellularDistanceFunction>(currentDistanceFunction);
                    layer.noise.SetCellularDistanceFunction(layer.cellularDistanceFunction);
                    needsRegeneration = true;
                }

                // Cellular Return Type
                const char* cellularReturnTypes[] = { "CellValue", "Distance", "Distance2", "Distance2Add", "Distance2Sub", "Distance2Mul", "Distance2Div" };
                int currentReturnType = static_cast<int>(layer.cellularReturnType);
                if (ImGui::Combo(("Cellular Return Type##" + std::to_string(i)).c_str(), &currentReturnType, cellularReturnTypes, IM_ARRAYSIZE(cellularReturnTypes))) {
                    layer.cellularReturnType = static_cast<FastNoiseLite::CellularReturnType>(currentReturnType);
                    layer.noise.SetCellularReturnType(layer.cellularReturnType);
                    needsRegeneration = true;
                }

                // Cellular Jitter Slider
                if (ImGui::SliderFloat(("Cellular Jitter##" + std::to_string(i)).c_str(), &layer.cellularJitter, 0.0f, 1.0f, "%.2f")) {
                    layer.noise.SetCellularJitter(layer.cellularJitter);
                    needsRegeneration = true;
                }
            }

            // Add similar conditional blocks for other noise types if needed
        }
    }

    ImGui::Separator();

    // Threshold for Land/Water
    if (ImGui::SliderFloat("Land Threshold", &chunkManager.landThreshold, 0.3f, 0.7f, "%.2f")) {
        needsRegeneration = true;
    }

    // Border width
    if (ImGui::SliderFloat("Mask Border Width", &chunkManager.borderWidth, 0.0f, 50.0f, "%.2f")) {
        needsRegeneration = true;
    }

    // Attenuation factor
    if (ImGui::SliderFloat("Mask Attenuation Factor", &chunkManager.attenuationFactor, 0.1f, 4.0f, "%.05f")) {
        needsRegeneration = true;
    }

    // Optionally, display current frame rate
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    // Button to add a new noise layer
    if (ImGui::Button("Add Noise Layer")) {
        // Example: Adding a new Perlin noise layer
        NoiseLayer newLayer(FastNoiseLite::NoiseType_Perlin, 0.01f, 1.0f, 2022);
        newLayer.configureNoise();
        chunkManager.noiseLayers.push_back(newLayer);
        needsRegeneration = true;
    }

    // Button to remove the last noise layer
    if (ImGui::Button("Remove Last Noise Layer") && !chunkManager.noiseLayers.empty()) {
        chunkManager.noiseLayers.pop_back();
        needsRegeneration = true;
    }

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
