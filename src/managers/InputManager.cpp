// InputManager.cpp
#include "InputManager.h"
#include <imgui.h>

// Implementation of ZoomCommand
ZoomCommand::ZoomCommand(std::shared_ptr<Camera> camera, float factor)
    : m_camera(camera), m_factor(factor) {}

void ZoomCommand::execute() {
    DEBUG_DEBUG("ZoomCommand: Zooming camera by factor ", m_factor);
    m_camera->Zoom(m_factor);
}

// Implementation of PanCommand
PanCommand::PanCommand(std::shared_ptr<Camera> camera, sf::Vector2f direction)
    : m_camera(camera), m_direction(direction) {}

void PanCommand::execute() {
    DEBUG_DEBUG("PanCommand: Panning camera by direction ", m_direction.x, ", ", m_direction.y);
    m_camera->Move(m_direction);
}

// Implementation of PlaceCommand
PlaceCommand::PlaceCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map)
    : m_camera(camera), m_window(window), m_map(map) {}

void PlaceCommand::execute() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);
    sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos, m_camera->GetView());
    DEBUG_DEBUG("PlaceCommand: Attempting to place city at world position ", worldPos.x, ", ", worldPos.y);
    m_map->AddCity(worldPos);
}

// Implementation of DrawCommand
DrawCommand::DrawCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map)
    : m_camera(camera), m_window(window), m_map(map) {}

void DrawCommand::execute() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);
    sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos, m_camera->GetView());
    DEBUG_DEBUG("DrawCommand: Attempting to use the line tool at world position ", worldPos.x, ", ", worldPos.y);
    m_map->UseLineMode(worldPos);
}

SelectCommand::SelectCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map)
    : m_camera(camera), m_window(window), m_map(map) {}

void SelectCommand::execute() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);
    sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos, m_camera->GetView());
    DEBUG_DEBUG("SelectCommand: Attempting to select object at world position ", worldPos.x, ", ", worldPos.y);
    m_map->SelectObject(worldPos);
}

MoveCommand::MoveCommand(std::shared_ptr<Camera> camera, sf::RenderWindow& window, std::shared_ptr<Map> map)
    : m_camera(camera), m_window(window), m_map(map) {}

void MoveCommand::execute() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);
    sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos, m_camera->GetView());
    DEBUG_DEBUG("MoveCommand: Attempting to move handle to world position ", worldPos.x, ", ", worldPos.y);
    m_map->MoveSelectedLineHandle(worldPos);
}

// Implementation of InputManager
InputManager::InputManager(std::shared_ptr<EventManager> eventMgr,
    std::shared_ptr<StateManager> stateMgr,
    sf::RenderWindow& win,
    std::shared_ptr<Camera> camera,
    std::shared_ptr<Map> map)
    : m_eventManager(eventMgr)
    , m_stateManager(stateMgr)
    , m_window(win)
    , m_camera(camera)
    , m_map(map)
    , m_isDragging(false)
    , m_lastMousePos()
    , m_config{}
{
    DEBUG_INFO("InputManager: Initializing InputManager");
    m_stateManager->Subscribe("CurrentTool", [this](const std::any& data) {
        if (data.has_value()) {
            auto value = std::any_cast<std::string>(data);
            CheckSubscriptions();
            DEBUG_DEBUG("InputManager: Listening for CurrentTool state change.");
        }
        });
    InitializeSubscriptions();
    InitializeCommands();
}

void InputManager::HandleInput(float deltaTime) {
    // Handle zoom
    float mouseWheel = ImGui::GetIO().MouseWheel;
    if (mouseWheel != 0.0f) {
        DEBUG_VERBOSE("InputManager: Mouse wheel event detected: ", mouseWheel);
        ExecuteCommand(mouseWheel > 0.0f ? InputAction::ZoomOut : InputAction::ZoomIn);
    }

    // Handle movement
    for (const auto& [key, action] : m_keyMappings) {
        if (sf::Keyboard::isKeyPressed(key)) {
            DEBUG_VERBOSE("InputManager: Key pressed: ", key);
            ExecuteCommand(action);
        }
    }

    // Handle mouse dragging
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f currentMousePos = sf::Vector2f(ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        sf::Vector2f delta = currentMousePos - m_lastMousePos;
        float distanceSquared = delta.x * delta.x + delta.y * delta.y;
        float thresholdSquared = m_dragThreshold * m_dragThreshold;

        if (!m_isDragging && distanceSquared >= thresholdSquared) {
            m_isDragging = true;
            DEBUG_DEBUG("InputManager: Dragging started.");
        }

        if (m_isDragging) {
            ExecuteCommand(InputAction::Move);
        }

        m_lastMousePos = currentMousePos;
    }
}

void InputManager::SetConfig(const InputConfig& config) {
    DEBUG_INFO("InputManager: Setting new input configuration.");
    DEBUG_DEBUG("InputManager: Zoom Speed: ", config.zoomSpeed, ", Pan Speed: ", config.panSpeed);
    m_config = config;

    // Update zoom commands with new speed
    if (m_commands.find(InputAction::ZoomIn) != m_commands.end()) {
        m_commands[InputAction::ZoomIn] = std::make_unique<ZoomCommand>(m_camera, m_config.zoomSpeed);
    }
    if (m_commands.find(InputAction::ZoomOut) != m_commands.end()) {
        m_commands[InputAction::ZoomOut] = std::make_unique<ZoomCommand>(m_camera, 1.0f / m_config.zoomSpeed);
    }
}

const InputConfig& InputManager::GetConfig() const {
    return m_config;
}

void InputManager::InitializeSubscriptions() {
    // State Manager
    if (m_stateManager->GetState<std::string>("CurrentTool") == std::string("Place")) {
        DEBUG_DEBUG("InputManager: Subscribing to PlaceSubscription...");
        placeSubscription = m_eventManager->Subscribe(EventType::MouseButtonPressed,
            [this](const EventData& data) {
                if (std::holds_alternative<sf::Event>(data)) {
                    const sf::Event& event = std::get<sf::Event>(data);

                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.button == sf::Mouse::Right) {
                            DEBUG_DEBUG("InputManager: Right mouse button clicked for Place.");
                            ExecuteCommand(InputAction::Place);
                        }
                    }
                }
            }
        );
    }

    if (m_stateManager->GetState<std::string>("CurrentTool") == std::string("Line")) {
        DEBUG_DEBUG("InputManager: Subscribing to LineSubscription...");
        drawSubscription = m_eventManager->Subscribe(EventType::MouseButtonPressed,
            [this](const EventData& data) {
                if (std::holds_alternative<sf::Event>(data)) {
                    const sf::Event& event = std::get<sf::Event>(data);

                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.button == sf::Mouse::Right) {
                            DEBUG_DEBUG("InputManager: Right mouse button clicked for Line.");
                            ExecuteCommand(InputAction::Draw);
                        }
                    }
                }
            }
        );
    }

    // Add mouse button released subscription
    m_eventManager->Subscribe(EventType::MouseButtonReleased,
        [this](const EventData& data) {
            if (std::holds_alternative<sf::Event>(data)) {
                const sf::Event& event = std::get<sf::Event>(data);
                if (event.mouseButton.button == sf::Mouse::Left) {
                    m_isDragging = false;
                    DEBUG_DEBUG("InputManager: Left mouse button released. Dragging stopped.");
                }
            }
        }
    );

    // Modify existing MouseButtonPressed subscription for selection
    m_eventManager->Subscribe(EventType::MouseButtonPressed,
        [this](const EventData& data) {
            if (std::holds_alternative<sf::Event>(data)) {
                const sf::Event& event = std::get<sf::Event>(data);
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        ExecuteCommand(InputAction::Select);
                        m_isDragging = false;
                        m_lastMousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                        DEBUG_DEBUG("InputManager: Left mouse button pressed for selection.");
                    }
                }
            }
        }
    );
}

void InputManager::CheckSubscriptions() {
    if (m_stateManager->GetState<std::string>("CurrentTool") == std::string("Place")) {
        DEBUG_DEBUG("InputManager: Subscribing to PlaceSubscription...");
        placeSubscription = m_eventManager->Subscribe(EventType::MouseButtonPressed,
            [this](const EventData& data) {
                if (std::holds_alternative<sf::Event>(data)) {
                    const sf::Event& event = std::get<sf::Event>(data);

                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.button == sf::Mouse::Right) {
                            DEBUG_DEBUG("InputManager: Right mouse button clicked for Place.");
                            ExecuteCommand(InputAction::Place);
                        }
                    }
                }
            }
        );
    }
    else {
        DEBUG_DEBUG("InputManager: Unsubscribing from PlaceSubscription...");
        m_eventManager->Unsubscribe(placeSubscription);
    }

    if (m_stateManager->GetState<std::string>("CurrentTool") == std::string("Line")) {
        DEBUG_DEBUG("InputManager: Subscribing to LineSubscription...");
        drawSubscription = m_eventManager->Subscribe(EventType::MouseButtonPressed,
            [this](const EventData& data) {
                if (std::holds_alternative<sf::Event>(data)) {
                    const sf::Event& event = std::get<sf::Event>(data);

                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.button == sf::Mouse::Right) {
                            DEBUG_DEBUG("InputManager: Right mouse button clicked for Line.");
                            ExecuteCommand(InputAction::Draw);
                        }
                    }
                }
            }
        );
    }
    else {
        DEBUG_DEBUG("InputManager: Unsubscribing from LineSubscription...");
        m_eventManager->Unsubscribe(drawSubscription);
    }
}

void InputManager::InitializeCommands() {
    DEBUG_INFO("InputManager: Initializing input commands.");
    // Initialize zoom commands
    m_commands[InputAction::ZoomIn] = std::make_unique<ZoomCommand>(m_camera, m_config.zoomSpeed);
    m_commands[InputAction::ZoomOut] = std::make_unique<ZoomCommand>(m_camera, 1.0f / m_config.zoomSpeed);
    DEBUG_DEBUG("InputManager: Zoom commands initialized.");

    // Initialize pan commands
    m_commands[InputAction::PanLeft] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(-1, 0));
    m_commands[InputAction::PanRight] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(1, 0));
    m_commands[InputAction::PanUp] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(0, -1));
    m_commands[InputAction::PanDown] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(0, 1));
    DEBUG_DEBUG("InputManager: Pan commands initialized.");

    // Initialize tool commands
    m_commands[InputAction::Place] = std::make_unique<PlaceCommand>(m_camera, m_window, m_map);
    m_commands[InputAction::Draw] = std::make_unique<DrawCommand>(m_camera, m_window, m_map);
    m_commands[InputAction::Select] = std::make_unique<SelectCommand>(m_camera, m_window, m_map);
    m_commands[InputAction::Move] = std::make_unique<MoveCommand>(m_camera, m_window, m_map);
    DEBUG_DEBUG("InputManager: Tool commands initialized.");

    // Initialize key mappings
    m_keyMappings = {
        {sf::Keyboard::Left, InputAction::PanLeft},
        {sf::Keyboard::A, InputAction::PanLeft},
        {sf::Keyboard::Right, InputAction::PanRight},
        {sf::Keyboard::D, InputAction::PanRight},
        {sf::Keyboard::Up, InputAction::PanUp},
        {sf::Keyboard::W, InputAction::PanUp},
        {sf::Keyboard::Down, InputAction::PanDown},
        {sf::Keyboard::S, InputAction::PanDown}
    };
    DEBUG_DEBUG("InputManager: Key mappings initialized.");
}

void InputManager::ExecuteCommand(InputAction action) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    DEBUG_DEBUG("InputManager: Executing command for action: ", static_cast<int>(action));
    if (auto it = m_commands.find(action); it != m_commands.end()) {
        it->second->execute();
    }
    else {
        DEBUG_WARNING("InputManager: No command found for action: ", static_cast<int>(action));
    }
}
