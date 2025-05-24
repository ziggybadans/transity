#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../input/InteractionMode.h"
#include "../event/LineEvents.h"
#include "FastNoiseLite.h"
#include "../world/WorldGenerationSystem.h"

class UI {
public:
    UI(sf::RenderWindow& window, WorldGenerationSystem* worldGenSystem);
    ~UI();
    void initialize();
    void processEvent(const sf::Event& event);
    void update(sf::Time deltaTime, size_t numStationsInActiveLine);
    void renderFrame();
    void cleanupResources();
    InteractionMode getInteractionMode() const;

    const std::vector<FinalizeLineEvent>& getUiEvents() const;
    void clearUiEvents();

    bool getVisualizeNoiseState() const { return _visualizeNoise; }

private:
    sf::RenderWindow& m_window;
    InteractionMode m_currentInteractionMode;
    std::vector<FinalizeLineEvent> m_uiEvents;

    int _worldGenSeed;
    float _worldGenFrequency;
    int _worldGenNoiseType;
    int _worldGenFractalType;
    int _worldGenOctaves;
    float _worldGenLacunarity;
    float _worldGenGain;

    bool _visualizeNoise;
    bool _autoRegenerate;

    WorldGenerationSystem* _worldGenerationSystem;
};