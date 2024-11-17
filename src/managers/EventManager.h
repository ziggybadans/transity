#pragma once

#include <functional>
#include <map>
#include <vector>
#include <SFML/Window/Event.hpp>

// Enum representing different types of events.
enum class EventType {
    Closed,
    Resized,
    KeyPressed,
    MouseMoved,
    MouseWheelScrolled,
    MouseButtonPressed,
    MouseButtonReleased,
    // Add other event types as needed
    None // Represents no specific event
};

class EventManager {
public:
    using EventCallback = std::function<void(const sf::Event&)>;

    // Subscribes a callback function to a specific type of event.
    void Subscribe(EventType type, EventCallback callback) {
        listeners[type].emplace_back(callback);
    }

    // Dispatches an event to all the listeners that have subscribed to the corresponding event type.
    void Dispatch(const sf::Event& event) {
        EventType type = ConvertSFMLToEventType(event);
        if (type == EventType::None) {
            return; // Ignore unhandled events
        }
        // Call all the registered callbacks for the given event type.
        for (auto& callback : listeners[type]) {
            callback(event);
        }
    }

private:
    // Map storing a vector of callback functions for each event type.
    std::map<EventType, std::vector<EventCallback>> listeners;

    // Converts an SFML event to the corresponding EventType.
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
        case sf::Event::MouseButtonPressed:
            return EventType::MouseButtonPressed;
        case sf::Event::MouseButtonReleased:
            return EventType::MouseButtonReleased;
        default:
            return EventType::None; // No action for unhandled events
        }
    }
};
