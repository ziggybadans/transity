#pragma once

#include "../managers/InputManager.h"
#include "../graphics/Camera.h"
#include "Constants.h"
#include <functional>
#include <iostream>
#include <memory>

class ActionRegistrar {
public:
    ActionRegistrar(std::shared_ptr<InputManager> inputManager, std::shared_ptr<Camera> camera)
        : m_inputManager(inputManager), m_camera(camera) {}

    void RegisterActions() {
        m_inputManager->RegisterActionCallback(InputAction::ZoomIn, [this]() {
            try {
                m_camera->Zoom(m_inputManager->GetZoomSpeed());
                std::cout << "Zoomed In. Current Zoom Level: " << m_camera->GetZoomLevel() << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error during Zoom In: " << e.what() << std::endl;
            }
        });

        m_inputManager->RegisterActionCallback(InputAction::ZoomOut, [this]() {
            try {
                m_camera->Zoom(1.0f / m_inputManager->GetZoomSpeed());
                std::cout << "Zoomed Out. Current Zoom Level: " << m_camera->GetZoomLevel() << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error during Zoom Out: " << e.what() << std::endl;
            }
        });

        m_inputManager->RegisterActionCallback(InputAction::PanLeft, [this]() {
            float zoom = m_camera->GetZoomLevel();
            float scaledPanSpeed = m_inputManager->GetPanSpeed() * zoom;
            m_camera->Move(sf::Vector2f(-scaledPanSpeed, 0.0f));
            std::cout << "Panning Left." << std::endl;
        });

        m_inputManager->RegisterActionCallback(InputAction::PanRight, [this]() {
            float zoom = m_camera->GetZoomLevel();
            float scaledPanSpeed = m_inputManager->GetPanSpeed() * zoom;
            m_camera->Move(sf::Vector2f(scaledPanSpeed, 0.0f));
            std::cout << "Panning Right." << std::endl;
        });

        m_inputManager->RegisterActionCallback(InputAction::PanUp, [this]() {
            float zoom = m_camera->GetZoomLevel();
            float scaledPanSpeed = m_inputManager->GetPanSpeed() * zoom;
            m_camera->Move(sf::Vector2f(0.0f, -scaledPanSpeed));
            std::cout << "Panning Up." << std::endl;
        });

        m_inputManager->RegisterActionCallback(InputAction::PanDown, [this]() {
            float zoom = m_camera->GetZoomLevel();
            float scaledPanSpeed = m_inputManager->GetPanSpeed() * zoom;
            m_camera->Move(sf::Vector2f(0.0f, scaledPanSpeed));
            std::cout << "Panning Down." << std::endl;
        });
    }

private:
    std::shared_ptr<InputManager> m_inputManager;
    std::shared_ptr<Camera> m_camera;
}; 