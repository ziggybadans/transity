#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <thread>
#include "transity/core/time_manager.hpp"

using namespace transity::core;
using namespace std::chrono;
using namespace std::chrono_literals;

TEST_CASE("TimeManager initialization", "[core][time]") {
    TimeManager timeManager;
    
    SECTION("Initial state") {
        CHECK(timeManager.getTimeScale() == 1.0f);
        CHECK(timeManager.getDeltaTime() == 0.0);
        CHECK(timeManager.getUnscaledDeltaTime() == 0.0);
        CHECK(timeManager.getCurrentTick() == 0);
        CHECK(timeManager.getTickProgress() == 0.0);
    }
}

TEST_CASE("TimeManager update and delta time", "[core][time]") {
    TimeManager timeManager;
    
    SECTION("Delta time calculation") {
        std::this_thread::sleep_for(16ms);
        timeManager.update();
        
        CHECK(timeManager.getDeltaTime() > 0.0);
        CHECK(timeManager.getUnscaledDeltaTime() > 0.0);
        CHECK(timeManager.getDeltaTime() == timeManager.getUnscaledDeltaTime());
    }
    
    SECTION("Time scale affects delta time") {
        timeManager.setTimeScale(2.0f);
        std::this_thread::sleep_for(16ms);
        timeManager.update();
        
        CHECK(timeManager.getTimeScale() == 2.0f);
        CHECK_THAT(timeManager.getDeltaTime(), 
                  Catch::Matchers::WithinRel(timeManager.getUnscaledDeltaTime() * 2.0, 0.0001));
    }
}

TEST_CASE("TimeManager fixed timestep", "[core][time]") {
    TimeManager timeManager;
    
    SECTION("Tick accumulation") {
        // Sleep for enough time to accumulate multiple ticks
        std::this_thread::sleep_for(100ms);
        timeManager.update();
        
        CHECK(timeManager.getCurrentTick() > 0);
        CHECK(timeManager.getTickProgress() >= 0.0);
        CHECK(timeManager.getTickProgress() < 1.0);
    }
    
    SECTION("Fixed timestep consistency") {
        CHECK_THAT(timeManager.getFixedDeltaTime(), 
                  Catch::Matchers::WithinRel(1.0 / 60.0, 0.0001));
    }
}

TEST_CASE("TimeManager event scheduling", "[core][time]") {
    TimeManager timeManager;
    bool eventTriggered = false;
    
    SECTION("Event execution") {
        // Schedule an event to run after 100ms
        timeManager.scheduleEvent(100ms, [&eventTriggered]() { eventTriggered = true; });
        
        // Initial state
        CHECK_FALSE(eventTriggered);
        
        // Wait and update until just before event time
        std::this_thread::sleep_for(50ms);
        timeManager.update();
        CHECK_FALSE(eventTriggered);
        
        // Wait and update after event time
        std::this_thread::sleep_for(100ms);
        timeManager.update();
        CHECK(eventTriggered);
    }
    
    SECTION("Event clearing") {
        timeManager.scheduleEvent(100ms, [&eventTriggered]() { eventTriggered = true; });
        timeManager.clearScheduledEvents();
        
        std::this_thread::sleep_for(150ms);
        timeManager.update();
        CHECK_FALSE(eventTriggered);
    }
}

TEST_CASE("TimeManager reset", "[core][time]") {
    TimeManager timeManager;
    
    // Set up some state
    timeManager.setTimeScale(2.0f);
    std::this_thread::sleep_for(32ms);
    timeManager.update();
    bool eventTriggered = false;
    timeManager.scheduleEvent(100ms, [&eventTriggered]() { eventTriggered = true; });
    
    // Reset
    timeManager.reset();
    
    SECTION("State after reset") {
        CHECK(timeManager.getTimeScale() == 2.0f); // Time scale should not be reset
        CHECK(timeManager.getDeltaTime() == 0.0);
        CHECK(timeManager.getUnscaledDeltaTime() == 0.0);
        CHECK(timeManager.getCurrentTick() == 0);
        CHECK(timeManager.getTickProgress() == 0.0);
        
        // Wait to ensure scheduled event was cleared
        std::this_thread::sleep_for(150ms);
        timeManager.update();
        CHECK_FALSE(eventTriggered);
    }
}

TEST_CASE("TimeManager high precision measurements", "[core][time]") {
    TimeManager timeManager;
    
    SECTION("High precision time initialization") {
        CHECK(timeManager.getPreciseFrameDuration() == TimeManager::Duration::zero());
        CHECK(timeManager.getHighResTickCount() == 0);
        CHECK(timeManager.getTimeSinceStart() >= 0.0);
    }
    
    SECTION("High precision time updates") {
        auto initialTime = timeManager.getHighResTimePoint();
        std::this_thread::sleep_for(16ms);
        timeManager.update();
        
        // Check that time has advanced
        CHECK(timeManager.getHighResTimePoint() > initialTime);
        CHECK(timeManager.getPreciseFrameDuration() > TimeManager::Duration::zero());
        CHECK(timeManager.getHighResTickCount() > 0);
        
        // Verify time since start is consistent
        double timeSinceStart = timeManager.getTimeSinceStart();
        CHECK(timeSinceStart > 0.0);
        CHECK(timeSinceStart < 1.0); // Should be less than 1 second in this test
    }
    
    SECTION("High precision time reset") {
        std::this_thread::sleep_for(16ms);
        timeManager.update();
        timeManager.reset();
        
        CHECK(timeManager.getPreciseFrameDuration() == TimeManager::Duration::zero());
        CHECK(timeManager.getHighResTickCount() == 0);
        CHECK(timeManager.getTimeSinceStart() >= 0.0);
    }
    
    SECTION("High precision frame duration accuracy") {
        std::this_thread::sleep_for(100ms);
        timeManager.update();
        
        auto frameDuration = duration_cast<milliseconds>(timeManager.getPreciseFrameDuration()).count();
        CHECK(frameDuration >= 95); // Allow for some system timing variance
        CHECK(frameDuration <= 115); // Increased tolerance for system scheduling
    }
} 