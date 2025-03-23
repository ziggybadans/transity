#pragma once

#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>
#include <typeindex>
#include <vector>
#include "system.hpp"

namespace transity::core {

// Forward declarations
class EventBase {
public:
    virtual ~EventBase() = default;
    virtual std::type_index getType() const = 0;
};

template<typename T>
class Event : public EventBase {
public:
    T data;
    explicit Event(const T& eventData) : data(eventData) {}
    std::type_index getType() const override { return std::type_index(typeid(T)); }
};

struct EventQueueEntry {
    std::shared_ptr<EventBase> event;
    int priority;
    size_t sequence;

    bool operator<(const EventQueueEntry& other) const {
        if (priority != other.priority) return priority < other.priority;
        return sequence > other.sequence;
    }
};

class EventManager : public ISystem {
public:
    EventManager();
    ~EventManager() override = default;

    // ISystem interface implementation
    bool initialize() override;
    void update(float deltaTime) override;
    void shutdown() override;
    std::string getName() const override { return "EventManager"; }

    // Event management
    template<typename T>
    void publish(const T& eventData, int priority = 0) {
        auto event = std::make_shared<Event<T>>(eventData);
        eventQueue.push({event, priority, ++sequenceNumber});
    }

    template<typename T>
    void subscribe(std::function<void(const T&)> handler) {
        auto typeIndex = std::type_index(typeid(T));
        handlers[typeIndex].push_back(
            [handler](const EventBase& e) {
                handler(static_cast<const Event<T>&>(e).data);
            }
        );
    }

    // Debug and analysis
    void enableEventLogging(bool enable);
    void clearEventQueue();
    size_t getQueueSize() const;
    void replayEvents(const std::vector<std::shared_ptr<EventBase>>& events);

private:
    std::priority_queue<EventQueueEntry> eventQueue;
    std::unordered_map<std::type_index, std::vector<std::function<void(const EventBase&)>>> handlers;
    size_t sequenceNumber;
    bool eventLoggingEnabled;
    std::vector<std::shared_ptr<EventBase>> eventLog;

    void processCurrentEvents();
    void logEvent(const std::shared_ptr<EventBase>& event);
};

} // namespace transity::core 