#include "transity/core/event_manager.hpp"
#include "transity/core/debug_manager.hpp"
#include <spdlog/spdlog.h>

namespace transity::core {

EventManager::EventManager() 
    : sequenceNumber(0)
    , eventLoggingEnabled(false) {
}

bool EventManager::initialize() {
    spdlog::info("Initializing Event Manager");
    return true;
}

void EventManager::update(float deltaTime) {
    processCurrentEvents();
}

void EventManager::shutdown() {
    spdlog::info("Shutting down Event Manager");
    clearEventQueue();
    handlers.clear();
    eventLog.clear();
}

void EventManager::processCurrentEvents() {
    // Process all events in the current frame
    while (!eventQueue.empty()) {
        auto entry = eventQueue.top();
        eventQueue.pop();

        if (eventLoggingEnabled) {
            logEvent(entry.event);
        }

        auto typeIndex = entry.event->getType();
        auto it = handlers.find(typeIndex);
        
        if (it != handlers.end()) {
            for (const auto& handler : it->second) {
                try {
                    handler(*entry.event);
                } catch (const std::exception& e) {
                    spdlog::error("Error processing event: {}", e.what());
                }
            }
        }
    }
}

void EventManager::enableEventLogging(bool enable) {
    eventLoggingEnabled = enable;
    if (enable) {
        spdlog::info("Event logging enabled");
    } else {
        spdlog::info("Event logging disabled");
        eventLog.clear();
    }
}

void EventManager::clearEventQueue() {
    std::priority_queue<EventQueueEntry> empty;
    std::swap(eventQueue, empty);
}

size_t EventManager::getQueueSize() const {
    return eventQueue.size();
}

void EventManager::logEvent(const std::shared_ptr<EventBase>& event) {
    eventLog.push_back(event);
    spdlog::debug("Event logged: type={}", event->getType().name());
}

void EventManager::replayEvents(const std::vector<std::shared_ptr<EventBase>>& events) {
    spdlog::info("Replaying {} events", events.size());
    for (const auto& event : events) {
        auto typeIndex = event->getType();
        auto it = handlers.find(typeIndex);
        
        if (it != handlers.end()) {
            for (const auto& handler : it->second) {
                try {
                    handler(*event);
                } catch (const std::exception& e) {
                    spdlog::error("Error replaying event: {}", e.what());
                }
            }
        }
    }
    spdlog::info("Event replay completed");
}

} // namespace transity::core 