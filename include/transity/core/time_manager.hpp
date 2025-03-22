#pragma once

#include <chrono>
#include <functional>
#include <queue>
#include <vector>

namespace transity::core {

class TimeManager {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;
    using ScheduledEvent = std::pair<TimePoint, std::function<void()>>;

    TimeManager();
    ~TimeManager() = default;

    // Delete copy and move operations
    TimeManager(const TimeManager&) = delete;
    TimeManager& operator=(const TimeManager&) = delete;
    TimeManager(TimeManager&&) = delete;
    TimeManager& operator=(TimeManager&&) = delete;

    // Core time management
    void update();
    void reset();
    
    // Time scale control
    void setTimeScale(float scale);
    float getTimeScale() const;

    // Delta time access
    double getDeltaTime() const;
    double getFixedDeltaTime() const;
    double getUnscaledDeltaTime() const;

    // Game tick management
    uint64_t getCurrentTick() const;
    double getTickProgress() const;

    // Event scheduling
    void scheduleEvent(Duration delay, std::function<void()> event);
    void clearScheduledEvents();

private:
    void updateScheduledEvents();

    // Time tracking
    TimePoint m_lastFrameTime;
    TimePoint m_gameStartTime;
    double m_deltaTime;
    double m_unscaledDeltaTime;
    float m_timeScale;

    // Fixed timestep
    static constexpr double FIXED_TIMESTEP = 1.0 / 60.0;
    double m_accumulatedTime;
    uint64_t m_currentTick;

    // Event scheduling
    struct EventCompare {
        bool operator()(const ScheduledEvent& a, const ScheduledEvent& b) const {
            return a.first > b.first;
        }
    };
    std::priority_queue<ScheduledEvent, std::vector<ScheduledEvent>, EventCompare> m_scheduledEvents;
};

} // namespace transity::core 