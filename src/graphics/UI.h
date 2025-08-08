#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../event/EventBus.h"
#include "../event/LineEvents.h"
#include "FastNoiseLite.h"
#include "../world/WorldGenerationSystem.h"
#include "../core/GameState.h"

class TerrainRenderSystem;

class UI {
public:
    UI(sf::RenderWindow& window, entt::registry& registry, WorldGenerationSystem* worldGenSystem, TerrainRenderSystem* terrainRenderSystem, GameState& gameState, EventBus& eventBus);
    ~UI();
    void initialize();
    void processEvent(const sf::Event& event);
    void update(sf::Time deltaTime, size_t numStationsInActiveLine);
    void renderFrame();
    void cleanupResources();

private:
    sf::RenderWindow& _window;
    GameState& _gameState;
    EventBus& _eventBus;
    entt::registry& _registry;

    bool _autoRegenerate;

    WorldGenerationSystem* _worldGenerationSystem;
    TerrainRenderSystem* _terrainRenderSystem;
    
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
};
