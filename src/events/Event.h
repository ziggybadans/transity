// src/events/Event.h
#pragma once
enum class EventType {
    PlayerMoved,
    // Add other event types
};

struct Event {
    EventType type;
    // Additional event data
};
