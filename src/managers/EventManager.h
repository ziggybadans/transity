// EventManager.h
#pragma once
#include <functional>
#include <map>
#include <vector>
#include <SFML/Window/Event.hpp>

enum class EventType {
    Closed,
    Resized,
    KeyPressed,
    MouseMoved,
    MouseWheelScrolled,
    // Add other event types as needed
    None // Represents no specific event
};

class EventManager {
public:
    using EventCallback = std::function<void(const sf::Event&)>;

    void Subscribe(EventType type, EventCallback callback) {
        listeners[type].emplace_back(callback);
    }

    void Dispatch(const sf::Event& event) {
        EventType type = ConvertSFMLToEventType(event);
        if (type == EventType::None) {
            return; // Ignore unhandled events
        }
        for (auto& callback : listeners[type]) {
            callback(event);
        }
    }

private:
    std::map<EventType, std::vector<EventCallback>> listeners;

    EventType ConvertSFMLToEventType(const sf::Event& event) {
        switch (event.type) {
        case sf::Event::Closed:
            return EventType::Closed;
        case sf::Event::Resized:
            return EventType::Resized;
        case sf::Event::KeyPressed:
            return EventType::KeyPressed;
        case sf::Event::MouseMoved:
            return EventType::MouseMoved;
        case sf::Event::MouseWheelScrolled:
            return EventType::MouseWheelScrolled;
            // Add mappings for other event types as needed
        default:
            return EventType::None; // No action for unhandled events
        }
    }
};
