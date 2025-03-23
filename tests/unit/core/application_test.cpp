#include <catch2/catch_test_macros.hpp>
#include "transity/core/application.hpp"
#include <thread>
#include <future>

using namespace transity::core;

TEST_CASE("Application initialization and shutdown", "[core][application]") {
    auto& app = Application::getInstance();
    
    SECTION("Application starts uninitialized") {
        REQUIRE_FALSE(app.isInitialized());
        REQUIRE(app.getAppName().empty());
    }
    
    SECTION("Application can be initialized with default name") {
        REQUIRE_NOTHROW(app.initialize());
        REQUIRE(app.isInitialized());
        REQUIRE(app.getAppName() == "Transity");
        app.shutdown();
    }
    
    SECTION("Application can be initialized with custom name") {
        const std::string customName = "TestApp";
        REQUIRE_NOTHROW(app.initialize(customName));
        REQUIRE(app.isInitialized());
        REQUIRE(app.getAppName() == customName);
        app.shutdown();
    }
    
    SECTION("Double initialization throws exception") {
        app.initialize();
        REQUIRE_THROWS_AS(app.initialize(), InitializationError);
        app.shutdown();
    }
    
    SECTION("Shutdown can be called multiple times safely") {
        app.initialize();
        REQUIRE_NOTHROW(app.shutdown());
        REQUIRE_FALSE(app.isInitialized());
        REQUIRE_NOTHROW(app.shutdown());
    }
    
    SECTION("Singleton pattern works correctly") {
        auto& app1 = Application::getInstance();
        auto& app2 = Application::getInstance();
        REQUIRE(&app1 == &app2);
    }
}

TEST_CASE("Application run state", "[core][application]") {
    auto& app = Application::getInstance();
    
    SECTION("Run without initialization throws exception") {
        REQUIRE_FALSE(app.isInitialized());
        REQUIRE_THROWS_AS(app.run(), StateError);
    }
    
    SECTION("Run after initialization") {
        app.initialize();
        
        // Run the application in a separate thread
        auto future = std::async(std::launch::async, [&app]() {
            app.run();
        });

        // Give it a moment to start up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Stop the application
        app.stop();
        
        // Wait for the game loop to finish (with timeout)
        auto status = future.wait_for(std::chrono::seconds(2));
        REQUIRE(status == std::future_status::ready);
        
        app.shutdown();
    }
}

TEST_CASE("Game state management", "[core][application]") {
    auto& app = Application::getInstance();
    app.initialize();

    SECTION("Initial state is Running after initialization") {
        REQUIRE(app.getGameState() == Application::GameState::Running);
    }

    SECTION("Pause and resume functionality") {
        REQUIRE(app.getGameState() == Application::GameState::Running);
        
        REQUIRE_NOTHROW(app.pause());
        REQUIRE(app.getGameState() == Application::GameState::Paused);
        
        REQUIRE_NOTHROW(app.resume());
        REQUIRE(app.getGameState() == Application::GameState::Running);
    }

    SECTION("Stop functionality") {
        REQUIRE(app.getGameState() == Application::GameState::Running);
        
        REQUIRE_NOTHROW(app.stop());
        REQUIRE(app.getGameState() == Application::GameState::Stopped);
    }

    SECTION("Cannot pause when not running") {
        app.stop();
        REQUIRE(app.getGameState() == Application::GameState::Stopped);
        
        REQUIRE_THROWS_AS(app.pause(), StateError);
        REQUIRE(app.getGameState() == Application::GameState::Stopped);
    }

    SECTION("Cannot resume when not paused") {
        REQUIRE(app.getGameState() == Application::GameState::Running);
        
        REQUIRE_THROWS_AS(app.resume(), StateError);
        REQUIRE(app.getGameState() == Application::GameState::Running);
    }

    app.shutdown();
}

TEST_CASE("FPS control", "[core][application]") {
    auto& app = Application::getInstance();
    app.initialize();

    SECTION("Default FPS settings") {
        REQUIRE(app.getCurrentFPS() == 0.0f);
    }

    SECTION("Set target FPS") {
        const unsigned int targetFPS = 30;
        REQUIRE_NOTHROW(app.setTargetFPS(targetFPS));
    }

    SECTION("Invalid FPS values") {
        REQUIRE_THROWS_AS(app.setTargetFPS(1001), StateError);
    }

    app.shutdown();
} 