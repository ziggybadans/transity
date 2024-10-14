// src/states/GameplayState.h
#pragma once
#include "../core/State.h"
#include "../systems/Renderer.h"
#include "../entities/EntityManager.h"
#include "../systems/TerrainGenerationSystem.h"
#include <memory>

class GameplayState : public State {
public:
    GameplayState(Game& game);
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render() override;

private:
    EntityManager entityManager;
    std::shared_ptr<Renderer> renderer;
    std::unique_ptr<TerrainGenerationSystem> terrainSystem;
    int worldWidth = 1000;  // Example size
    int worldHeight = 1000;
    float scale = 0.1f;      // Noise scale
};
