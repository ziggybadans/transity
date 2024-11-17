#pragma once

#include "../managers/InputManager.h"
#include "../graphics/Camera.h"
#include "Constants.h"
#include "../Debug.h"
#include <memory>

class ActionRegistrar {
public:
    ActionRegistrar(std::shared_ptr<InputManager> inputManager, std::shared_ptr<Camera> camera)
        : m_inputManager(inputManager), m_camera(camera) {
        DEBUG_INFO("ActionRegistrar initialized");
    }

    void RegisterActions() {
        m_inputManager->RegisterActionCallback(InputAction::ZoomIn, [this]() {
            try {
                m_camera->Zoom(m_inputManager->GetZoomSpeed());
                DEBUG_DEBUG("Camera zoomed in to level: ", m_camera->GetZoomLevel());
            }
            catch (const std::exception& e) {
                DEBUG_ERROR("Failed to zoom in: ", e.what());
            }
        });

        m_inputManager->RegisterActionCallback(InputAction::ZoomOut, [this]() {
            try {
                m_camera->Zoom(1.0f / m_inputManager->GetZoomSpeed());
                DEBUG_DEBUG("Camera zoomed out to level: ", m_camera->GetZoomLevel());
            }
            catch (const std::exception& e) {
                DEBUG_ERROR("Failed to zoom out: ", e.what());
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

        DEBUG_INFO("Action callbacks registered successfully");
    }

private:
    std::shared_ptr<InputManager> m_inputManager;
    std::shared_ptr<Camera> m_camera;
}; 