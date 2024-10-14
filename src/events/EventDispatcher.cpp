// src/events/EventDispatcher.cpp
#include "EventDispatcher.h"

void EventDispatcher::subscribe(EventType type, EventCallback callback) {
    listeners[type].push_back(callback);
}

void EventDispatcher::dispatch(const Event& event) {
    auto it = listeners.find(event.type);
    if (it != listeners.end()) {
        for (auto& callback : it->second) {
            callback(event);
        }
    }
}
