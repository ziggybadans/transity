#pragma once

#include <functional>
#include <map>
#include <vector>
#include <variant>
#include <cstddef>
#include <algorithm>
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
    using SubscriptionID = std::size_t;

    EventManager() : nextId(1) {}

    // Subscribes a callback function to a specific type of event.
    SubscriptionID Subscribe(EventType type, EventCallback callback) {
        SubscriptionID id = nextId++;
        listeners[type].emplace_back(id, std::move(callback));
        idToEventType[id] = type;
        return id;
    }

    // Unsubscribes a previously subscribed callback using its SubscriptionID.
    bool Unsubscribe(SubscriptionID id) {
        auto it = idToEventType.find(id);
        if (it == idToEventType.end()) {
            return false; // ID not found
        }

        EventType type = it->second;
        auto& vec = listeners[type];
        // Remove the callback with the matching ID
        auto vecIt = std::remove_if(vec.begin(), vec.end(),
            [id](const Listener& listener) { return listener.id == id; });
        if (vecIt != vec.end()) {
            vec.erase(vecIt, vec.end());
            idToEventType.erase(it);
            return true;
        }
        return false;
    }

    // Dispatches an event to all the listeners that have subscribed to the corresponding event type.
    void Dispatch(const sf::Event& event) {
        EventType type = ConvertSFMLToEventType(event);
        if (type == EventType::None) {
            return; // Ignore unhandled events
        }
        EventData data = event; // Wrap the SFML event in the variant
        // Call all the registered callbacks for the given event type.
        for (auto& listener : listeners[type]) {
            listener.callback(data);
        }
    }

    // Dispatches a custom event to all the listeners that have subscribed to the corresponding event type.
    void Dispatch(EventType type, const ToolChangedEvent& customEvent) {
        if (type != EventType::ToolChanged) {
            return; // Extend this if handling more custom events
        }
        EventData data = customEvent; // Wrap the custom event in the variant
        for (auto& listener : listeners[type]) {
            listener.callback(data);
        }
    }

private:
    struct Listener {
        SubscriptionID id;
        EventCallback callback;

        Listener(SubscriptionID listenerId, EventCallback cb)
            : id(listenerId), callback(std::move(cb)) {}
    };

    // Map storing a vector of callback functions for each event type.
    std::map<EventType, std::vector<Listener>> listeners;

    std::map<SubscriptionID, EventType> idToEventType;

    SubscriptionID nextId;

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
