#include "transity/core/time_manager.hpp"
#include <chrono>

using namespace transity::core;
using namespace std::chrono;

TimeManager::TimeManager()
    : m_lastFrameTime(Clock::now())
    , m_gameStartTime(Clock::now())
    , m_deltaTime(0.0)
    , m_unscaledDeltaTime(0.0)
    , m_timeScale(1.0f)
    , m_accumulatedTime(0.0)
    , m_currentTick(0)
    , m_highResLastFrame(HighResClock::now())
    , m_preciseFrameDuration(Duration::zero())
    , m_highResTickCount(0) {
}

void TimeManager::update() {
    // Standard time update
    TimePoint currentTime = Clock::now();
    Duration frameDuration = currentTime - m_lastFrameTime;
    m_lastFrameTime = currentTime;

    // High precision time update
    HighResTimePoint currentHighResTime = HighResClock::now();
    m_preciseFrameDuration = currentHighResTime - m_highResLastFrame;
    m_highResLastFrame = currentHighResTime;
    m_highResTickCount = duration_cast<nanoseconds>(Clock::now() - m_gameStartTime).count();

    // Calculate unscaled delta time
    m_unscaledDeltaTime = duration_cast<duration<double>>(m_preciseFrameDuration).count();
    
    // Apply time scale to get scaled delta time
    m_deltaTime = m_unscaledDeltaTime * m_timeScale;

    // Update fixed timestep accumulator
    m_accumulatedTime += m_deltaTime;
    while (m_accumulatedTime >= FIXED_TIMESTEP) {
        m_accumulatedTime -= FIXED_TIMESTEP;
        m_currentTick++;
    }

    // Process scheduled events
    updateScheduledEvents();
}

void TimeManager::reset() {
    auto now = Clock::now();
    auto highResNow = HighResClock::now();
    m_gameStartTime = now;
    m_lastFrameTime = now;
    m_highResLastFrame = highResNow;
    m_deltaTime = 0.0;
    m_unscaledDeltaTime = 0.0;
    m_accumulatedTime = 0.0;
    m_currentTick = 0;
    m_preciseFrameDuration = Duration::zero();
    m_highResTickCount = 0;
    clearScheduledEvents();
}

void TimeManager::setTimeScale(float scale) {
    m_timeScale = scale;
}

float TimeManager::getTimeScale() const {
    return m_timeScale;
}

double TimeManager::getDeltaTime() const {
    return m_deltaTime;
}

double TimeManager::getFixedDeltaTime() const {
    return FIXED_TIMESTEP;
}

double TimeManager::getUnscaledDeltaTime() const {
    return m_unscaledDeltaTime;
}

uint64_t TimeManager::getCurrentTick() const {
    return m_currentTick;
}

double TimeManager::getTickProgress() const {
    return m_accumulatedTime / FIXED_TIMESTEP;
}

void TimeManager::scheduleEvent(Duration delay, std::function<void()> event) {
    TimePoint eventTime = Clock::now() + delay;
    m_scheduledEvents.emplace(eventTime, std::move(event));
}

void TimeManager::clearScheduledEvents() {
    while (!m_scheduledEvents.empty()) {
        m_scheduledEvents.pop();
    }
}

void TimeManager::updateScheduledEvents() {
    TimePoint currentTime = Clock::now();
    
    while (!m_scheduledEvents.empty()) {
        const auto& nextEvent = m_scheduledEvents.top();
        if (nextEvent.first <= currentTime) {
            nextEvent.second(); // Execute the event
            m_scheduledEvents.pop();
        } else {
            break;
        }
    }
}

double TimeManager::getTimeSinceStart() const {
    return duration_cast<duration<double>>(Clock::now() - m_gameStartTime).count();
}

TimeManager::HighResTimePoint TimeManager::getHighResTimePoint() const {
    return HighResClock::now();
}

TimeManager::Duration TimeManager::getPreciseFrameDuration() const {
    return m_preciseFrameDuration;
}

uint64_t TimeManager::getHighResTickCount() const {
    return m_highResTickCount;
} 