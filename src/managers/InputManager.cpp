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
    , m_config {}
{
    DEBUG_DEBUG("InputManager: Initializing InputManager");
    m_stateManager->Subscribe("CurrentTool", [this](const std::any& data) {
        if (data.has_value()) {
            auto value = std::any_cast<std::string>(data);
            CheckSubscriptions();
            DEBUG_DEBUG("InputManager: Listening for CurrentTool state change.");
        }
    });
    CheckSubscriptions();
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
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && m_stateManager->GetState<std::string>("CurrentTool") != std::string("TrainPlace")) {
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

EventManager::SubscriptionID InputManager::AddMouseSubscription(
    sf::Event::EventType type,
    sf::Mouse::Button button,
    InputAction action,
    MouseEventCallback callback
)
{
    EventManager::SubscriptionID subscription = m_eventManager->Subscribe(
        EventType::MouseButtonPressed,
        [this, button, action, callback](const EventData& data)
        {
            if (std::holds_alternative<sf::Event>(data)) {
                const sf::Event& event = std::get<sf::Event>(data);
                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == button)
                {
                    if (action != InputAction::None) {
                        // Call default "ExecuteCommand"
                        ExecuteCommand(action);
                    }

                    // Allow the user-provided callback to do whatever else is needed
                    if (callback) {
                        callback(event);
                    }
                }
            }
        }
    );

    return subscription;
}

void InputManager::CheckSubscriptions() {
    DEBUG_DEBUG("Checking subscriptions...");

    m_eventManager->Unsubscribe(placeSubscription);
    m_eventManager->Unsubscribe(drawSubscription);
    m_eventManager->Unsubscribe(trainPlaceLeftSubscription);
    m_eventManager->Unsubscribe(trainPlaceRightSubscription);
    m_eventManager->Unsubscribe(dragPressSubscription);
    m_eventManager->Unsubscribe(dragReleaseSubscription);

    std::string currentTool = m_stateManager->GetState<std::string>("CurrentTool");

    if (currentTool == "Place") {
        DEBUG_DEBUG("Subscribing to Place...");
        placeSubscription = AddMouseSubscription(sf::Event::MouseButtonPressed, sf::Mouse::Right, InputAction::Place);
    }
    else if (currentTool == "Line") {
        DEBUG_DEBUG("Subscribing to Draw...");
        drawSubscription = AddMouseSubscription(sf::Event::MouseButtonPressed, sf::Mouse::Right, InputAction::Draw);
    }
    else if (currentTool == "TrainPlace") {
        DEBUG_DEBUG("Subscribing to TrainPlace...");
        trainPlaceLeftSubscription = AddMouseSubscription(sf::Event::MouseButtonPressed, sf::Mouse::Left, InputAction::TrainPlaceLeft);
        trainPlaceRightSubscription = AddMouseSubscription(sf::Event::MouseButtonPressed, sf::Mouse::Right, InputAction::TrainPlaceRight);
    }


    if (currentTool == "Place" || currentTool == "Line") {
        DEBUG_DEBUG("Sunscribing to dragging subscriptions...");
        dragReleaseSubscription = AddMouseSubscription(sf::Event::MouseButtonReleased, sf::Mouse::Left, InputAction::None,
            [this](const sf::Event& event) {
                m_isDragging = false;
            }
        );

        dragPressSubscription = AddMouseSubscription(sf::Event::MouseButtonPressed, sf::Mouse::Left, InputAction::Select,
            [this](const sf::Event& event) {
                m_isDragging = false;
                m_lastMousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                DEBUG_DEBUG("Firing select action...");
            }
        );
    }
}

void InputManager::InitializeCommands() {
    DEBUG_INFO("InputManager: Initializing input commands.");
    // Initialize zoom commands
    m_commands[InputAction::ZoomIn]  = std::make_unique<ZoomCommand>(m_camera, m_config.zoomSpeed);
    m_commands[InputAction::ZoomOut] = std::make_unique<ZoomCommand>(m_camera, 1.0f / m_config.zoomSpeed);
    DEBUG_DEBUG("InputManager: Zoom commands initialized.");

    // Initialize pan commands
    m_commands[InputAction::PanLeft]  = std::make_unique<PanCommand>(m_camera, sf::Vector2f(-1, 0));
    m_commands[InputAction::PanRight] = std::make_unique<PanCommand>(m_camera, sf::Vector2f(1, 0));
    m_commands[InputAction::PanUp]    = std::make_unique<PanCommand>(m_camera, sf::Vector2f(0, -1));
    m_commands[InputAction::PanDown]  = std::make_unique<PanCommand>(m_camera, sf::Vector2f(0, 1));
    DEBUG_DEBUG("InputManager: Pan commands initialized.");

    // Initialize tool commands
    m_commands[InputAction::Place] = std::make_unique<MapInteractionCommand>(
        m_camera,
        m_window,
        m_map,
        [](std::shared_ptr<Map> map, const sf::Vector2f& worldPos) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                DEBUG_DEBUG("PlaceCommand: Attempting to place node at world position ", worldPos.x, ", ", worldPos.y);
                map->AddGenericNode(worldPos);
            }
            else {
                DEBUG_DEBUG("PlaceCommand: Attempting to place city at world position ", worldPos.x, ", ", worldPos.y);
                map->AddCity(worldPos);
            }
        }
    );

    // Draw
    m_commands[InputAction::Draw] = std::make_unique<MapInteractionCommand>(
        m_camera,
        m_window,
        m_map,
        [](std::shared_ptr<Map> map, const sf::Vector2f& worldPos) {
            DEBUG_DEBUG("DrawCommand: Attempting to use line tool at world position ", worldPos.x, ", ", worldPos.y);
            map->UseLineMode(worldPos);
        }
    );

    // Select
    m_commands[InputAction::Select] = std::make_unique<MapInteractionCommand>(
        m_camera,
        m_window,
        m_map,
        [](std::shared_ptr<Map> map, const sf::Vector2f& worldPos) {
            DEBUG_DEBUG("SelectCommand: Attempting to select object at world position ", worldPos.x, ", ", worldPos.y);
            map->SelectObject(worldPos);
        }
    );

    // Move
    m_commands[InputAction::Move] = std::make_unique<MapInteractionCommand>(
        m_camera,
        m_window,
        m_map,
        [](std::shared_ptr<Map> map, const sf::Vector2f& worldPos) {
            DEBUG_DEBUG("MoveCommand: Attempting to move handle to world position ", worldPos.x, ", ", worldPos.y);
            if (map->isLineSelected() && map->GetSelectedLine()->GetSelectedHandleIndex() != -1) {
                DEBUG_DEBUG("MoveCommand: Moving line handle to world position ", worldPos.x, ", ", worldPos.y);
                map->MoveSelectedLineHandle(worldPos);
            }
            // Otherwise, if a city is selected, attempt to move the city
            else {
                City* selectedCity = map->GetSelectionManager().GetSelectedCity();
                if (selectedCity) {
                    DEBUG_DEBUG("MoveCommand: Moving city to world position ", worldPos.x, ", ", worldPos.y);
                    map->MoveCity(worldPos);
                }
            }
        }
    );

    // TrainPlace
    m_commands[InputAction::TrainPlaceLeft] = std::make_unique<MapInteractionCommand>(
        m_camera,
        m_window,
        m_map,
        [](std::shared_ptr<Map> map, const sf::Vector2f& worldPos) {
            DEBUG_DEBUG("MoveCommand: Attempting to use train place mode at ", worldPos.x, ", ", worldPos.y);
            map->UseTrainPlaceMode(worldPos, true);
        }
    );
    m_commands[InputAction::TrainPlaceRight] = std::make_unique<MapInteractionCommand>(
        m_camera,
        m_window,
        m_map,
        [](std::shared_ptr<Map> map, const sf::Vector2f& worldPos) {
            DEBUG_DEBUG("MoveCommand: Attempting to use train place mode at ", worldPos.x, ", ", worldPos.y);
            map->UseTrainPlaceMode(worldPos, false);
        }
    );
    DEBUG_DEBUG("InputManager: Tool commands initialized.");

    // Initialize key mappings
    m_keyMappings = {
        {sf::Keyboard::Left,  InputAction::PanLeft},
        {sf::Keyboard::A,     InputAction::PanLeft},
        {sf::Keyboard::Right, InputAction::PanRight},
        {sf::Keyboard::D,     InputAction::PanRight},
        {sf::Keyboard::Up,    InputAction::PanUp},
        {sf::Keyboard::W,     InputAction::PanUp},
        {sf::Keyboard::Down,  InputAction::PanDown},
        {sf::Keyboard::S,     InputAction::PanDown}
    };
    DEBUG_DEBUG("InputManager: Key mappings initialized.");
}

void InputManager::ExecuteCommand(InputAction action) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (!m_window.hasFocus()) {
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
