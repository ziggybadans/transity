// src/events/EventDispatcher.h
#pragma once
#include "Event.h"  // Include the Event class to work with events
#include <functional> // Include for using std::function as callbacks
#include <map>        // Include for storing the event listeners
#include <vector>     // Include for storing multiple callbacks for each event type

class EventDispatcher {
public:
    using EventCallback = std::function<void(const Event&)>;  // Define a callback type for event handling

    // Subscribe a callback to a specific event type
    void subscribe(EventType type, EventCallback callback);

    // Dispatch an event to all listeners subscribed to its type
    void dispatch(const Event& event);

private:
    // Map of event types to their respective list of listeners (callbacks)
    std::map<EventType, std::vector<EventCallback>> listeners;
};

// Summary:
// The EventDispatcher class provides a mechanism for subscribing callbacks to specific event types and dispatching
// those events to all registered listeners. It maintains a map of event types to lists of callbacks, which allows
// multiple systems to react to game events in a decoupled and flexible way. This design supports extensibility
// by allowing different systems to subscribe to events without tight coupling between components.