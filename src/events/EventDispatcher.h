// src/events/EventDispatcher.h
#pragma once
#include "Event.h"
#include <functional>
#include <map>
#include <vector>

class EventDispatcher {
public:
    using EventCallback = std::function<void(const Event&)>;

    void subscribe(EventType type, EventCallback callback);
    void dispatch(const Event& event);

private:
    std::map<EventType, std::vector<EventCallback>> listeners;
};
