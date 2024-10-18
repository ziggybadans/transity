// src/Game.cpp
#include "Game.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <cmath>
#include <algorithm>

// Constructor
Game::Game(int chunkSize, int tileSize, int worldChunksX, int worldChunksY, const std::string& heightMapPath)
    : CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    WORLD_CHUNKS_X(worldChunksX), WORLD_CHUNKS_Y(worldChunksY),
    chunkManager(worldChunksX, worldChunksY, chunkSize, tileSize),
    threadPool(std::thread::hardware_concurrency()), // Initialize thread pool
    view(sf::FloatRect(0, 0, 800, 600)), // Default view size; will be updated
    inputHandler(view, sf::Vector2f(800, 600)), // Default window size; will be updated
    renderer(window, view, chunkManager, chunkSize, tileSize, sf::Vector2f(800, 600)),
    windowSizeX(2560), windowSizeY(1440), // Default window size; can be made configurable
    renderDistance(2)
{
    initializeWindow();

    if (!heightMapPath.empty()) {
        loadHeightMap(heightMapPath);
    }

    // Pass heightMap to ChunkManager
    chunkManager.setHeightMap(heightMap);

    // Calculate world size in pixels
    float worldSizeX = static_cast<float>(chunkManager.getWorldChunksX() * CHUNK_SIZE * TILE_SIZE);
    float worldSizeY = static_cast<float>(chunkManager.getWorldChunksY() * CHUNK_SIZE * TILE_SIZE);

    // Center the view
    view.setCenter(worldSizeX / 2.0f, worldSizeY / 2.0f);
    window.setView(view);

    // Dynamically calculate minZoomFactor based on window and world size
    float zoomX = static_cast<float>(windowSizeX) / worldSizeX;
    float zoomY = static_cast<float>(windowSizeY) / worldSizeY;
    inputHandler.setMinZoom(std::min(zoomX, zoomY)); // Ensure entire world fits within the view when zoomed out

    // Initialize activeChunks based on the initial view
    updateVisibleChunks();
}

// Destructor
Game::~Game() {
    ImGui::SFML::Shutdown();
}

// Initialize window and ImGui
void Game::initializeWindow() {
    // Create a window with a specified resolution
    window.create(sf::VideoMode(windowSizeX, windowSizeY), "2D Transport Game", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(120);

    // Initialize ImGui
    if (!ImGui::SFML::Init(window)) {
        std::cerr << "Failed to initialize ImGui-SFML.\n";
    }
}

// Load HeightMap
void Game::loadHeightMap(const std::string& heightMapPath) {
    try {
        heightMap = std::make_shared<HeightMap>(heightMapPath);
        std::cout << "Heightmap loaded successfully.\n";
        useRealHeightMap = true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load heightmap: " << e.what() << "\nProceeding with procedural generation.\n";
        heightMap = nullptr;
        useRealHeightMap = false;
    }
}

// Main game loop
void Game::run() {
    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Time deltaTime = deltaClock.restart();

        // Handle input events
        inputHandler.processEvents(window, deltaTime);
        if (inputHandler.shouldClose()) {
            window.close();
        }

        ImGui::SFML::Update(window, deltaTime);
        drawDebugGUI();

        // Handle regeneration if needed
        if (needsRegeneration && regenClock.getElapsedTime() > regenDelay) {
            chunkManager.regenerateAllChunks();
            needsRegeneration = false;
        }

        // Wrap the view within world boundaries
        wrapView();

        // Update visible chunks based on the new view
        updateVisibleChunks();

        // Render the frame
        renderer.renderFrame();
    }
}

// Wrap view to stay within world boundaries
void Game::wrapView() {
    float worldSizeX = static_cast<float>(chunkManager.getWorldChunksX() * CHUNK_SIZE * TILE_SIZE);
    float worldSizeY = static_cast<float>(chunkManager.getWorldChunksY() * CHUNK_SIZE * TILE_SIZE);

    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    float halfViewWidth = size.x / 2.0f;
    float halfViewHeight = size.y / 2.0f;

    // Clamp x-coordinate
    center.x = std::clamp(center.x, halfViewWidth, worldSizeX - halfViewWidth);

    // Clamp y-coordinate
    center.y = std::clamp(center.y, halfViewHeight, worldSizeY - halfViewHeight);

    view.setCenter(center);
    window.setView(view);
}

// In Game.cpp, modify updateVisibleChunks to batch chunk loading
void Game::updateVisibleChunks() {
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    int centerChunkX = static_cast<int>(center.x) / (CHUNK_SIZE * TILE_SIZE);
    int centerChunkY = static_cast<int>(center.y) / (CHUNK_SIZE * TILE_SIZE);

    int loadRadius = renderDistance;

    std::unordered_set<ChunkCoord> newActiveChunks;
    for (int y = centerChunkY - loadRadius; y <= centerChunkY + loadRadius; ++y) {
        for (int x = centerChunkX - loadRadius; x <= centerChunkX + loadRadius; ++x) {
            if (x < 0 || x >= chunkManager.getWorldChunksX() || y < 0 || y >= chunkManager.getWorldChunksY())
                continue;
            newActiveChunks.emplace(ChunkCoord{ x, y });
        }
    }

    std::vector<ChunkCoord> chunksToUnload;
    std::vector<ChunkCoord> chunksToLoad;

    {
        std::lock_guard<std::mutex> lock(activeChunksMutex);
        for (const auto& chunk : activeChunks) {
            if (newActiveChunks.find(chunk) == newActiveChunks.end()) {
                chunksToUnload.push_back(chunk);
            }
        }
        for (const auto& coord : newActiveChunks) {
            if (activeChunks.find(coord) == activeChunks.end() && !chunkManager.isChunkLoaded(coord.x, coord.y)) {
                chunksToLoad.push_back(coord);
            }
        }
        activeChunks = std::move(newActiveChunks);
    }

    for (const auto& coord : chunksToUnload) {
        chunkManager.unloadChunk(coord.x, coord.y);
    }

    // Batch loading chunks
    const size_t BATCH_SIZE = 10; // Example batch size
    for (size_t i = 0; i < chunksToLoad.size(); i += BATCH_SIZE) {
        size_t end = std::min(i + BATCH_SIZE, chunksToLoad.size());
        std::vector<ChunkCoord> batch(chunksToLoad.begin() + i, chunksToLoad.begin() + end);
        threadPool.enqueue([this, batch]() {
            for (const auto& coord : batch) {
                auto chunk = chunkManager.generateChunk(coord.x, coord.y);
                chunkManager.addLoadedChunk(coord.x, coord.y, chunk);
            }
            });
    }
}

// Convert world position to chunk coordinates
ChunkCoord Game::getChunkCoordFromPosition(const sf::Vector2f& position) const {
    int chunkX = static_cast<int>(position.x) / (CHUNK_SIZE * TILE_SIZE);
    int chunkY = static_cast<int>(position.y) / (CHUNK_SIZE * TILE_SIZE);
    return ChunkCoord{ chunkX, chunkY };
}

// Draw Debug GUI using ImGui
void Game::drawDebugGUI() {
    ImGui::Begin("Debug Controls");

    // Generation Mode Selection
    static int generationMode = useRealHeightMap ? 1 : 0; // 0: Procedural, 1: HeightMap
    const char* modes[] = { "Procedural", "HeightMap" };
    if (ImGui::Combo("Generation Mode", &generationMode, modes, IM_ARRAYSIZE(modes))) {
        if (generationMode == 0) {
            chunkManager.enableProceduralGeneration();
            useRealHeightMap = false;
        }
        else if (generationMode == 1) {
            // Ideally, integrate a file dialog. For simplicity, using a fixed path or prompt the user.
            std::string heightMapPath = "assets/heightmaps/world_heightmap.png";
            chunkManager.enableHeightMapGeneration(heightMapPath);
            useRealHeightMap = true;
        }
        needsRegeneration = true;
        regenClock.restart();
    }

    // Display current settings
    ImGui::Text("World Generation Settings:");

    // Iterate through each noise layer for dynamic controls
    const auto& noiseLayers = chunkManager.getNoiseLayers();
    for (size_t i = 0; i < noiseLayers.size(); ++i) {
        const NoiseLayer& layer = noiseLayers[i];
        std::string layerLabel = "Noise Layer " + std::to_string(i + 1);
        if (ImGui::CollapsingHeader(layerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            // Noise Type Combo Box
            const char* noiseTypes[] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
            int currentNoiseType = static_cast<int>(layer.noiseType);
            if (ImGui::Combo(("Type##" + std::to_string(i)).c_str(), &currentNoiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes))) {
                chunkManager.setNoiseLayerType(i, static_cast<FastNoiseLite::NoiseType>(currentNoiseType));
                needsRegeneration = true;
                regenClock.restart();
            }

            // Frequency Slider
            float frequency = layer.frequency;
            if (ImGui::SliderFloat(("Frequency##" + std::to_string(i)).c_str(), &frequency, 0.001f, 0.1f, "%.5f")) {
                chunkManager.setNoiseLayerFrequency(i, frequency);
                needsRegeneration = true;
                regenClock.restart();
            }

            // Amplitude Slider
            float amplitude = layer.amplitude;
            if (ImGui::SliderFloat(("Amplitude##" + std::to_string(i)).c_str(), &amplitude, 0.1f, 5.0f, "%.2f")) {
                chunkManager.setNoiseLayerAmplitude(i, amplitude);
                needsRegeneration = true;
                regenClock.restart();
            }

            // Seed Input
            int seed = layer.seed;
            if (ImGui::InputInt(("Seed##" + std::to_string(i)).c_str(), &seed)) {
                chunkManager.setNoiseLayerSeed(i, seed);
                needsRegeneration = true;
                regenClock.restart();
            }

            // Additional parameters based on noise type
            if (layer.noiseType == FastNoiseLite::NoiseType_Cellular) {
                // Cellular Distance Function
                const char* cellularDistanceFunctions[] = { "Euclidean", "EuclideanSq", "Manhattan", "Hybrid" };
                int currentDistanceFunction = static_cast<int>(layer.cellularDistanceFunction);
                if (ImGui::Combo(("Cellular Distance Function##" + std::to_string(i)).c_str(), &currentDistanceFunction, cellularDistanceFunctions, IM_ARRAYSIZE(cellularDistanceFunctions))) {
                    chunkManager.setNoiseLayerCellularDistanceFunction(i, static_cast<FastNoiseLite::CellularDistanceFunction>(currentDistanceFunction));
                    needsRegeneration = true;
                    regenClock.restart();
                }

                // Cellular Return Type
                const char* cellularReturnTypes[] = { "CellValue", "Distance", "Distance2", "Distance2Add", "Distance2Sub", "Distance2Mul", "Distance2Div" };
                int currentReturnType = static_cast<int>(layer.cellularReturnType);
                if (ImGui::Combo(("Cellular Return Type##" + std::to_string(i)).c_str(), &currentReturnType, cellularReturnTypes, IM_ARRAYSIZE(cellularReturnTypes))) {
                    chunkManager.setNoiseLayerCellularReturnType(i, static_cast<FastNoiseLite::CellularReturnType>(currentReturnType));
                    needsRegeneration = true;
                    regenClock.restart();
                }

                // Cellular Jitter Slider
                float cellularJitter = layer.cellularJitter;
                if (ImGui::SliderFloat(("Cellular Jitter##" + std::to_string(i)).c_str(), &cellularJitter, 0.0f, 1.0f, "%.2f")) {
                    chunkManager.setNoiseLayerCellularJitter(i, cellularJitter);
                    needsRegeneration = true;
                    regenClock.restart();
                }
            }
        }
    }

    ImGui::Separator();

    // Threshold for Land/Water
    float landThreshold = chunkManager.getLandThreshold();
    if (ImGui::SliderFloat("Land Threshold", &landThreshold, 0.3f, 0.7f, "%.2f")) {
        chunkManager.setLandThreshold(landThreshold);
        needsRegeneration = true;
        regenClock.restart();
    }

    // Border width
    float borderWidth = chunkManager.getBorderWidth();
    if (ImGui::SliderFloat("Mask Border Width", &borderWidth, 0.0f, 50.0f, "%.2f")) {
        chunkManager.setBorderWidth(borderWidth);
        needsRegeneration = true;
        regenClock.restart();
    }

    // Attenuation factor
    float attenuationFactor = chunkManager.getAttenuationFactor();
    if (ImGui::SliderFloat("Mask Attenuation Factor", &attenuationFactor, 0.1f, 4.0f, "%.05f")) {
        chunkManager.setAttenuationFactor(attenuationFactor);
        needsRegeneration = true;
        regenClock.restart();
    }

    // Optionally, display current frame rate
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / std::max(ImGui::GetIO().Framerate, 1.0f), ImGui::GetIO().Framerate);

    // Button to add a new noise layer
    if (ImGui::Button("Add Noise Layer")) {
        // Example: Adding a new Perlin noise layer
        chunkManager.addNoiseLayer(NoiseLayer(FastNoiseLite::NoiseType_Perlin, 0.01f, 1.0f, 2022));
        needsRegeneration = true;
        regenClock.restart();
    }

    // Button to remove the last noise layer
    if (ImGui::Button("Remove Last Noise Layer") && chunkManager.getNoiseLayers().size() > 0) {
        chunkManager.removeLastNoiseLayer();
        needsRegeneration = true;
        regenClock.restart();
    }

    ImGui::End();
}
