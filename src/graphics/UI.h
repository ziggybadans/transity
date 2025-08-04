#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../event/EventBus.h" // <-- Add this
#include "../event/LineEvents.h"
#include "FastNoiseLite.h"
#include "../world/WorldGenerationSystem.h"
#include "../core/GameState.h"

class TerrainRenderSystem;

class UI {
public:
    // Constructor now takes EventBus
    UI(sf::RenderWindow& window, WorldGenerationSystem* worldGenSystem, TerrainRenderSystem* terrainRenderSystem, GameState& gameState, EventBus& eventBus);
    ~UI();
    void initialize();
    void processEvent(const sf::Event& event);
    void update(sf::Time deltaTime, size_t numStationsInActiveLine);
    void renderFrame();
    void cleanupResources();

    bool getVisualizeNoiseState() const { return _visualizeNoise; }

private:
    sf::RenderWindow& _window;
    GameState& _gameState;
    EventBus& _eventBus; // <-- Add this

    int _worldChunksX;
    int _worldChunksY;
    int _chunkSizeX;
    int _chunkSizeY;
    float _cellSize;

    bool _visualizeNoise;
    bool _autoRegenerate;

    WorldGenerationSystem* _worldGenerationSystem;
    TerrainRenderSystem* _terrainRenderSystem; // ADD

    void syncWithWorldState();
    
    bool _visualizeChunkBorders = false; // ADD
    bool _visualizeCellBorders = false;  // ADD
};
