#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../event/EventBus.h" // <-- Add this
#include "../event/LineEvents.h"
#include "FastNoiseLite.h"
#include "../world/WorldGenerationSystem.h"
#include "../core/GameState.h"

class UI {
public:
    // Constructor now takes EventBus
    UI(sf::RenderWindow& window, WorldGenerationSystem* worldGenSystem, GameState& gameState, EventBus& eventBus);
    ~UI();
    void initialize();
    void processEvent(const sf::Event& event);
    void update(sf::Time deltaTime, size_t numStationsInActiveLine);
    void renderFrame();
    void cleanupResources();

    // These methods are no longer needed
    // const std::vector<FinalizeLineEvent>& getUiEvents() const;
    // void clearUiEvents();

    bool getVisualizeNoiseState() const { return _visualizeNoise; }

private:
    sf::RenderWindow& _window;
    GameState& _gameState;
    EventBus& _eventBus; // <-- Add this

    // This vector is no longer needed
    // std::vector<FinalizeLineEvent> _uiEvents;

    WorldGenParams _worldGenParams;

    int _worldChunksX;
    int _worldChunksY;
    int _chunkSizeX;
    int _chunkSizeY;
    float _cellSize;

    bool _visualizeNoise;
    bool _autoRegenerate;

    WorldGenerationSystem* _worldGenerationSystem;

    void syncWithWorldState();
};
