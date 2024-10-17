// src/Game.cpp
#include "Game.h"
#include "Utilities.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <cmath>

// Constructor
Game::Game(int chunkSize, int tileSize, int WORLD_CHUNKS_X, int WORLD_CHUNKS_Y, const std::string& heightMapPath)
    : CHUNK_SIZE(chunkSize), TILE_SIZE(tileSize),
    chunkManager(WORLD_CHUNKS_X, WORLD_CHUNKS_Y, CHUNK_SIZE, TILE_SIZE),
    windowSizeX(2560), windowSizeY(1440),
    view(sf::FloatRect(0, 0, windowSizeX, windowSizeY)),
    inputHandler(view, sf::Vector2f(windowSizeX, windowSizeY)),
    renderer(window, view, chunkManager, CHUNK_SIZE, TILE_SIZE, sf::Vector2f(windowSizeX, windowSizeY)),
    heightMap(nullptr),
    renderDistance(2.0f)
{
    if (!heightMapPath.empty()) {
        try {
            heightMap = new HeightMap(heightMapPath);
            std::cout << "Heightmap loaded successfully.\n";
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << "\nProceeding with procedural generation.\n";
        }
    }

    // Pass heightMap to ChunkManager
    chunkManager.setHeightMap(heightMap); // You'll need to implement this method

    // Calculate world size in pixels
    float worldSizeX = chunkManager.WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = chunkManager.WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    // Create a window with a specified resolution
    window.create(sf::VideoMode(windowSizeX, windowSizeY), "2D Transport Game");
    window.setFramerateLimit(120);

    ImGui::SFML::Init(window);

    view.setCenter(worldSizeX / 2.0f, worldSizeY / 2.0f);

    // Dynamically calculate minZoomFactor based on window and world size
    float zoomX = static_cast<float>(windowSizeX) / worldSizeX;
    float zoomY = static_cast<float>(windowSizeY) / worldSizeY;
    inputHandler.minZoom = std::min(zoomX, zoomY); // Ensure entire world fits within the view when zoomed out

    // Initialize activeChunks based on the initial view
    updateVisibleChunks();
}

// Destructor
Game::~Game() {
    ImGui::SFML::Shutdown();
    delete heightMap;
}

// Main game loop
void Game::run() {
    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Time deltaTime = deltaClock.restart();

        inputHandler.processEvents(window, deltaTime);
        ImGui::SFML::Update(window, deltaTime);
        drawDebugGUI();

        if (needsRegeneration && regenClock.getElapsedTime() > regenDelay) {
            // Regenerate all loaded chunks
            std::unordered_map<ChunkCoord, Chunk> currentChunks = chunkManager.getLoadedChunks();
            for (const auto& [coord, _] : currentChunks) {
                chunkManager.regenerateChunk(coord.x, coord.y);
            }
            needsRegeneration = false;
        }

        wrapView();
        updateVisibleChunks(); // Update chunks based on the new view
        renderer.renderFrame();
    }
}

void Game::wrapView() {
    float worldSizeX = chunkManager.WORLD_CHUNKS_X * CHUNK_SIZE * TILE_SIZE;
    float worldSizeY = chunkManager.WORLD_CHUNKS_Y * CHUNK_SIZE * TILE_SIZE;

    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    float halfViewWidth = size.x / 2.0f;
    float halfViewHeight = size.y / 2.0f;

    // Clamp x-coordinate
    if (center.x - halfViewWidth < 0.0f)
        center.x = halfViewWidth;
    if (center.x + halfViewWidth > worldSizeX)
        center.x = worldSizeX - halfViewWidth;

    // Clamp y-coordinate
    if (center.y - halfViewHeight < 0.0f)
        center.y = halfViewHeight;
    if (center.y + halfViewHeight > worldSizeY)
        center.y = worldSizeY - halfViewHeight;

    view.setCenter(center);
}

void Game::updateVisibleChunks() {
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    // Determine the range of chunks to load based on renderDistance
    int centerChunkX = static_cast<int>(center.x) / (CHUNK_SIZE * TILE_SIZE);
    int centerChunkY = static_cast<int>(center.y) / (CHUNK_SIZE * TILE_SIZE);

    int loadRadius = static_cast<int>(std::ceil(renderDistance));

    std::unordered_set<ChunkCoord, std::hash<ChunkCoord>> newActiveChunks;

    for (int y = centerChunkY - loadRadius; y <= centerChunkY + loadRadius; ++y) {
        for (int x = centerChunkX - loadRadius; x <= centerChunkX + loadRadius; ++x) {
            // Clamp chunk coordinates to world boundaries
            if (x < 0 || x >= chunkManager.WORLD_CHUNKS_X || y < 0 || y >= chunkManager.WORLD_CHUNKS_Y)
                continue;

            ChunkCoord coord{ x, y };
            newActiveChunks.insert(coord);

            // Load chunk if not already loaded
            {
                std::lock_guard<std::mutex> lock(activeChunksMutex);
                if (activeChunks.find(coord) == activeChunks.end() && !chunkManager.isChunkLoaded(x, y)) {
                    // Initiate asynchronous loading
                    loadingChunks.emplace_back(chunkManager.loadChunkAsync(x, y));
                }
            }
        }
    }

    // Unload chunks that are no longer active
    {
        std::lock_guard<std::mutex> lock(activeChunksMutex);
        for (auto it = activeChunks.begin(); it != activeChunks.end();) {
            if (newActiveChunks.find(*it) == newActiveChunks.end()) {
                chunkManager.unloadChunk(it->x, it->y);
                it = activeChunks.erase(it);
            }
            else {
                ++it;
            }
        }

        // Add new active chunks
        for (const auto& coord : newActiveChunks) {
            activeChunks.insert(coord);
        }
    }

    // Process completed chunk loading
    for (auto it = loadingChunks.begin(); it != loadingChunks.end();) {
        if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            try {
                Chunk loadedChunk = it->get();
                // The chunk is already stored in ChunkManager's loadedChunks map
            }
            catch (const std::exception& e) {
                std::cerr << "Error loading chunk: " << e.what() << "\n";
            }
            it = loadingChunks.erase(it);
        }
        else {
            ++it;
        }
    }
}

ChunkCoord Game::getChunkCoordFromPosition(const sf::Vector2f& position) const {
    int chunkX = static_cast<int>(position.x) / (CHUNK_SIZE * TILE_SIZE);
    int chunkY = static_cast<int>(position.y) / (CHUNK_SIZE * TILE_SIZE);
    return ChunkCoord{ chunkX, chunkY };
}

// Modified drawDebugGUI function
void Game::drawDebugGUI() {
    // Create a window for debug controls
    ImGui::Begin("Debug Controls");

    // Generation Mode Selection
    static int generationMode = 0; // 0: Procedural, 1: HeightMap
    const char* modes[] = { "Procedural", "HeightMap" };
    if (ImGui::Combo("Generation Mode", &generationMode, modes, IM_ARRAYSIZE(modes))) {
        if (generationMode == 0) {
            chunkManager.enableProceduralGeneration();
        }
        else if (generationMode == 1) {
            // Prompt user to input heightmap path
            // For simplicity, using a fixed path. Integrate a file dialog for flexibility.
            std::string heightMapPath = "assets/heightmaps/world_heightmap.png";
            chunkManager.enableHeightMapGeneration(heightMapPath);
        }
    }

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