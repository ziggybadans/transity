#pragma once

#include <functional>
#include <map>
#include <vector>
#include <variant>
#include <SFML/Window/Event.hpp>

// Enum representing different types of events.
enum class EventType {
    Closed,
    MouseButtonPressed,
    ToolChanged,
    None // Represents no specific event
};

struct ToolChangedEvent {
    std::string newTool; // Example data: the name of the new tool
};

// Define a variant that can hold either an SFML event or a custom event
using EventData = std::variant<sf::Event, ToolChangedEvent>;

class EventManager {
public:
    using EventCallback = std::function<void(const EventData&)>;

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
        EventData data = event; // Wrap the SFML event in the variant
        // Call all the registered callbacks for the given event type.
        for (auto& callback : listeners[type]) {
            callback(data);
        }
    }

    // Dispatches a custom event to all the listeners that have subscribed to the corresponding event type.
    void Dispatch(EventType type, const ToolChangedEvent& customEvent) {
        if (type != EventType::ToolChanged) {
            return; // Extend this if handling more custom events
        }
        EventData data = customEvent; // Wrap the custom event in the variant
        for (auto& callback : listeners[type]) {
            callback(data);
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
        case sf::Event::MouseButtonPressed:
            return EventType::MouseButtonPressed;
        default:
            return EventType::None; // No action for unhandled events
        }
    }
};
