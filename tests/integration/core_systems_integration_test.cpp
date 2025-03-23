#include <catch2/catch_test_macros.hpp>
#include "transity/core/application.hpp"
#include "transity/core/time_manager.hpp"
#include "transity/core/system_manager.hpp"
#include "transity/core/debug_manager.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace transity::core;

// Integration test between DebugManager and TimeManager
TEST_CASE("Integration between DebugManager and TimeManager", "[integration]") {
    auto& debug = DebugManager::getInstance();
    TimeManager timeManager;
    
    SECTION("TimeManager performance metrics are captured by DebugManager") {
        // Start profiling
        debug.startProfiling("time_manager_test");
        
        // Begin profiling a specific operation
        debug.beginProfile("time_update", "time_manager");
        
        // Run multiple updates to generate measurable time
        for (int i = 0; i < 10; ++i) {
            timeManager.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        debug.endProfile("time_update");
        debug.stopProfiling();
        
        // Verify profiling data exists
        auto events = debug.getProfilingEvents("time_manager");
        REQUIRE(!events.empty());
        REQUIRE(events[0].name == "time_update");
        REQUIRE(events[0].category == "time_manager");
        
        // Duration should be at least 50ms (10 iterations * 5ms sleep)
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            events[0].endTime - events[0].startTime).count();
        REQUIRE(duration >= 50);
    }
}

// Integration test between DebugManager and SystemManager
TEST_CASE("Integration between DebugManager and SystemManager", "[integration]") {
    class TestSystem : public ISystem {
    public:
        bool initCalled = false;
        bool updateCalled = false;
        bool shutdownCalled = false;
        
        bool initialize() override {
            initCalled = true;
            return true;
        }
        
        void update(float dt) override {
            updateCalled = true;
        }
        
        void shutdown() override {
            shutdownCalled = true;
        }
        
        std::string getName() const override {
            return "TestSystem";
        }
    };
    
    SystemManager systemManager;
    
    SECTION("System lifecycle events work correctly") {
        // Register our test system
        auto* system = systemManager.registerSystem<TestSystem>();
        REQUIRE(system != nullptr);
        
        // Manually initialize the system
        systemManager.initialize();
        
        // Now check if the system was initialized
        REQUIRE(system->initCalled);
        
        // Update the system
        systemManager.update(0.016f);
        REQUIRE(system->updateCalled);
        
        // Shutdown and verify
        systemManager.shutdown();
        REQUIRE(system->shutdownCalled);
    }
}

// Integration test between DebugManager and Application
TEST_CASE("Integration between DebugManager and Application", "[integration]") {
    auto& app = Application::getInstance();
    
    SECTION("Application lifecycle works correctly") {
        // Initialize app
        app.initialize("IntegrationTest");
        REQUIRE(app.isInitialized());
        
        // Test state transitions
        app.pause();
        REQUIRE(app.getGameState() == Application::GameState::Paused);
        
        app.resume();
        REQUIRE(app.getGameState() == Application::GameState::Running);
        
        app.stop();
        REQUIRE(app.getGameState() == Application::GameState::Stopped);
        
        // Shutdown
        app.shutdown();
        REQUIRE_FALSE(app.isInitialized());
    }
} 