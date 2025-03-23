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

    SECTION("Clear log history") {
        debug.log(LogLevel::Info, "Test message");
        REQUIRE(!debug.getLogHistory().empty());
        debug.clearLogHistory();
        REQUIRE(debug.getLogHistory().empty());
    }
}

TEST_CASE("DebugManager performance metrics", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Metric timing works") {
        debug.beginMetric("test_metric", "test_category", "ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        debug.endMetric("test_metric");
        
        const auto& metrics = debug.getMetrics();
        REQUIRE(!metrics.empty());
        REQUIRE(metrics.back().name == "test_metric");
        REQUIRE(metrics.back().category == "test_category");
        REQUIRE(metrics.back().unit == "ms");
        REQUIRE(metrics.back().value >= 10.0); // At least 10ms
    }

    SECTION("Get metrics by category") {
        debug.beginMetric("metric1", "category1", "ms");
        debug.endMetric("metric1");
        debug.beginMetric("metric2", "category2", "ms");
        debug.endMetric("metric2");

        auto category1Metrics = debug.getMetricsByCategory("category1");
        REQUIRE(category1Metrics.size() == 1);
        REQUIRE(category1Metrics[0].name == "metric1");

        auto category2Metrics = debug.getMetricsByCategory("category2");
        REQUIRE(category2Metrics.size() == 1);
        REQUIRE(category2Metrics[0].name == "metric2");
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

    SECTION("Debug sections work") {
        debug.createDebugSection("test_section");
        debug.addDebugInfoToSection("test_section", "test_key", "test_value");
        const auto& section = debug.getDebugSection("test_section");
        REQUIRE(section.at("test_key") == "test_value");
    }
}

TEST_CASE("DebugManager command system", "[debug]") {
    auto& debug = DebugManager::getInstance();
    bool commandExecuted = false;
    
    SECTION("Command registration and execution") {
        debug.registerCommand("test_cmd", 
            [&commandExecuted](const std::vector<std::string>& args) {
                commandExecuted = true;
                REQUIRE(args.size() == 2);
                REQUIRE(args[0] == "arg1");
                REQUIRE(args[1] == "arg2");
            },
            "Test command description"
        );
        
        REQUIRE(debug.executeCommand("test_cmd arg1 arg2"));
        REQUIRE(commandExecuted);

        // Check command description
        auto commands = debug.getCommandList();
        REQUIRE(commands["test_cmd"] == "Test command description");
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

    SECTION("Clear system state") {
        debug.setSystemState("test_system", "running");
        REQUIRE(debug.getSystemState("test_system") == "running");
        
        debug.clearSystemState("test_system");
        REQUIRE(debug.getSystemState("test_system") == "");
    }

    SECTION("Get all system states") {
        debug.setSystemState("system1", "running");
        debug.setSystemState("system2", "stopped");
        
        const auto& states = debug.getAllSystemStates();
        REQUIRE(states.size() == 2);
        REQUIRE(states.at("system1") == "running");
        REQUIRE(states.at("system2") == "stopped");
    }
}

TEST_CASE("DebugManager performance monitoring and profiling", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Profiling session management") {
        debug.startProfiling("test_session");
        REQUIRE(debug.getProfilingEvents().empty());
        
        debug.beginProfile("test_profile", "test_category");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        debug.endProfile("test_profile");
        
        debug.stopProfiling();
        auto events = debug.getProfilingEvents();
        REQUIRE(events.size() == 1);
        REQUIRE(events[0].name == "test_profile");
        REQUIRE(events[0].category == "test_category");
        REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(
            events[0].endTime - events[0].startTime).count() >= 10);
    }
    
    SECTION("Profile metadata") {
        debug.startProfiling("test_session");
        debug.beginProfile("test_profile", "test_category");
        debug.addProfileMetadata("test_profile", "test_key", "test_value");
        
        auto profiles = debug.getActiveProfiles();
        REQUIRE(profiles.size() == 1);
        REQUIRE(profiles[0].metadata["test_key"] == "test_value");
        
        debug.endProfile("test_profile");
        debug.stopProfiling();
    }
}

TEST_CASE("DebugManager memory tracking", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Memory usage tracking") {
        auto currentUsage = debug.getCurrentMemoryUsage();
        REQUIRE(currentUsage >= 0);
        
        debug.resetPeakMemoryUsage();
        REQUIRE(debug.getPeakMemoryUsage() == currentUsage);
    }
}

TEST_CASE("DebugManager system resource monitoring", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Resource monitoring toggle") {
        debug.enableResourceMonitoring(true);
        REQUIRE(debug.isResourceMonitoringEnabled());
        
        debug.enableResourceMonitoring(false);
        REQUIRE_FALSE(debug.isResourceMonitoringEnabled());
    }
    
    SECTION("Resource utilization") {
        auto metrics = debug.getResourceUtilization();
        REQUIRE(metrics.find("cpu") != metrics.end());
        REQUIRE(metrics.find("memory") != metrics.end());
        REQUIRE(metrics.find("gpu") != metrics.end());
        REQUIRE(metrics.find("disk") != metrics.end());
    }
}

TEST_CASE("DebugManager event replay system", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Event recording") {
        debug.startEventRecording();
        REQUIRE(debug.isRecordingEvents());
        
        std::map<std::string, std::string> eventData{{"key", "value"}};
        debug.recordEvent("test_event", eventData);
        
        debug.stopEventRecording();
        REQUIRE_FALSE(debug.isRecordingEvents());
        
        bool eventReplayed = false;
        debug.replayEvents([&](const std::string& type, const std::map<std::string, std::string>& data) {
            REQUIRE(type == "test_event");
            REQUIRE(data.at("key") == "value");
            eventReplayed = true;
        });
        REQUIRE(eventReplayed);
    }
    
    SECTION("Event log save/load") {
        debug.startEventRecording();
        std::map<std::string, std::string> eventData{{"key", "value"}};
        debug.recordEvent("test_event", eventData);
        debug.stopEventRecording();
        
        const std::string testFile = "test_events.json";
        debug.saveEventLog(testFile);
        
        // Clear recorded events by starting a new recording
        debug.startEventRecording();
        debug.stopEventRecording();
        
        debug.loadEventLog(testFile);
        bool eventLoaded = false;
        debug.replayEvents([&](const std::string& type, const std::map<std::string, std::string>& data) {
            REQUIRE(type == "test_event");
            REQUIRE(data.at("key") == "value");
            eventLoaded = true;
        });
        REQUIRE(eventLoaded);
    }
}

TEST_CASE("Debug Overlay", "[debug]") {
    auto& debug = DebugManager::getInstance();
    
    SECTION("Debug overlay visibility") {
        REQUIRE_FALSE(debug.isDebugOverlayEnabled());
        
        debug.setDebugOverlayEnabled(true);
        REQUIRE(debug.isDebugOverlayEnabled());
        
        debug.setDebugOverlayEnabled(false);
        REQUIRE_FALSE(debug.isDebugOverlayEnabled());
    }
    
    SECTION("Debug overlay content") {
        debug.setDebugOverlayEnabled(true);
        
        // Add metrics
        debug.addMetric("FPS", []() { return "60"; });
        debug.addMetric("Memory", []() { return "100MB"; });
        
        // Get overlay content
        auto content = debug.getDebugOverlayContent();
        REQUIRE(content.find("FPS: 60") != std::string::npos);
        REQUIRE(content.find("Memory: 100MB") != std::string::npos);
        
        // Remove metric
        debug.removeMetric("FPS");
        content = debug.getDebugOverlayContent();
        REQUIRE(content.find("FPS: 60") == std::string::npos);
        REQUIRE(content.find("Memory: 100MB") != std::string::npos);
    }
    
    SECTION("Debug sections") {
        debug.setDebugOverlayEnabled(true);
        
        // Add a collapsible section
        debug.beginSection("Performance");
        debug.addMetric("CPU Usage", []() { return "25%"; });
        debug.endSection();
        
        auto content = debug.getDebugOverlayContent();
        REQUIRE(content.find("Performance") != std::string::npos);
        REQUIRE(content.find("CPU Usage: 25%") != std::string::npos);
    }
    
    SECTION("Debug overlay update") {
        debug.setDebugOverlayEnabled(true);
        
        int counter = 0;
        debug.addMetric("Counter", [&counter]() { return std::to_string(counter++); });
        
        auto content1 = debug.getDebugOverlayContent();
        auto content2 = debug.getDebugOverlayContent();
        
        REQUIRE(content1 != content2); // Counter should have incremented
    }
} 