// src/events/EventDispatcher.cpp
#include "EventDispatcher.h"

// Adds a new callback to the list of listeners for a specific event type
void EventDispatcher::subscribe(EventType type, EventCallback callback) {
    listeners[type].push_back(callback); // Add the callback to the list for the given event type
}

// Dispatches an event to all registered listeners for the event's type
void EventDispatcher::dispatch(const Event& event) {
    auto it = listeners.find(event.type); // Find the listeners for the specific event type
    if (it != listeners.end()) { // If there are listeners registered for this event type
        for (auto& callback : it->second) { // Iterate over all the callbacks
            callback(event); // Call each listener with the event as the argument
        }
    }
}

// Summary:
// The EventDispatcher implementation handles the subscription of callbacks to different event types and the dispatching
// of events to those callbacks. The subscribe method allows listeners to register for specific event types, while the
// dispatch method invokes all registered callbacks for a given event, ensuring that relevant systems react to the event.