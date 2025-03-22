#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "transity/core/debug_manager.hpp"
#include <thread>
#include <chrono>

using namespace transity::core;

TEST_CASE("DebugManager logging functionality", "[debug]") {
    auto& debug = DebugManager::getInstance();
    debug.setLogLevel(LogLevel::Debug);

    SECTION("Log messages are stored in history") {
        debug.log(LogLevel::Info, "Test message");
        const auto& history = debug.getLogHistory();
        REQUIRE(!history.empty());
        REQUIRE(history.back().first == LogLevel::Info);
        REQUIRE(history.back().second == "Test message");
    }

    SECTION("Log level filtering works") {
        debug.setLogLevel(LogLevel::Warning);
        debug.log(LogLevel::Info, "Should not be logged");
        debug.log(LogLevel::Warning, "Should be logged");
        const auto& history = debug.getLogHistory();
        REQUIRE(history.back().first == LogLevel::Warning);
        REQUIRE(history.back().second == "Should be logged");
    }
}

TEST_CASE("DebugManager performance metrics", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Metric timing works") {
        debug.beginMetric("test_metric");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        debug.endMetric("test_metric");
        
        const auto& metrics = debug.getMetrics();
        REQUIRE(!metrics.empty());
        REQUIRE(metrics.back().name == "test_metric");
        REQUIRE(metrics.back().value >= 10.0); // At least 10ms
    }
}

TEST_CASE("DebugManager debug overlay", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Debug overlay toggle works") {
        debug.setDebugOverlayEnabled(true);
        REQUIRE(debug.isDebugOverlayEnabled());
        
        debug.setDebugOverlayEnabled(false);
        REQUIRE_FALSE(debug.isDebugOverlayEnabled());
    }
    
    SECTION("Debug info storage works") {
        debug.addDebugInfo("test_key", "test_value");
        const auto& info = debug.getDebugInfo();
        REQUIRE(info.at("test_key") == "test_value");
    }
}

TEST_CASE("DebugManager command system", "[debug]") {
    auto& debug = DebugManager::getInstance();
    bool commandExecuted = false;
    
    SECTION("Command registration and execution") {
        debug.registerCommand("test_cmd", [&commandExecuted](const std::vector<std::string>& args) {
            commandExecuted = true;
            REQUIRE(args.size() == 2);
            REQUIRE(args[0] == "arg1");
            REQUIRE(args[1] == "arg2");
        });
        
        REQUIRE(debug.executeCommand("test_cmd arg1 arg2"));
        REQUIRE(commandExecuted);
    }
    
    SECTION("Invalid command handling") {
        REQUIRE_FALSE(debug.executeCommand("nonexistent_cmd"));
    }
}

TEST_CASE("DebugManager system state", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("System state tracking") {
        debug.setSystemState("test_system", "running");
        REQUIRE(debug.getSystemState("test_system") == "running");
        
        debug.setSystemState("test_system", "stopped");
        REQUIRE(debug.getSystemState("test_system") == "stopped");
        
        REQUIRE(debug.getSystemState("nonexistent_system") == "");
    }
} 