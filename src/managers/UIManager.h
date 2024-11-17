#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include "EventManager.h"
#include "../interfaces/IInitializable.h"

class UIManager : public IInitializable {
public:
    UIManager();
    ~UIManager();

    /* Core UI Methods */
    bool Init() override;
    void ProcessEvent(const sf::Event& event);
    void Update(float deltaTime);
    void Render();
    void Shutdown();

    /* Setters */
    void SetWindow(sf::RenderWindow& window);

private:
    /* UI State */
    bool m_initialized;
    sf::RenderWindow* m_renderWindow;
    float* m_timeScalePtr;
    float m_fps;
};
