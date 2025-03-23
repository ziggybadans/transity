#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "transity/core/application.hpp"
#include "transity/core/input_manager.hpp"
#include "transity/core/time_manager.hpp"
#include "transity/core/system_manager.hpp"
#include "transity/core/debug_manager.hpp"
#include <chrono>
#include <random>
#include <thread>
#include <atomic>

using namespace transity::core;

TEST_CASE("Core Systems Performance Benchmarks", "[core][performance]") {
    auto& app = Application::getInstance();
    app.initialize("PerformanceTest");

    SECTION("Time Manager Performance") {
        TimeManager timeManager;
        
        BENCHMARK("Time Update") {
            timeManager.update();
            return timeManager.getDeltaTime();
        };

        BENCHMARK("Fixed Timestep Access") {
            return timeManager.getFixedDeltaTime();
        };

        BENCHMARK("Time Scale Control") {
            timeManager.setTimeScale(2.0f);
            return timeManager.getTimeScale();
        };
    }

    SECTION("Input Manager Performance") {
        auto& inputManager = InputManager::getInstance();
        
        BENCHMARK("Input State Updates") {
            inputManager.update();
            return inputManager.isKeyPressed(sf::Keyboard::A);
        };

        BENCHMARK("Mouse Position Queries") {
            return inputManager.getMousePosition();
        };
    }

    SECTION("System Manager Performance") {
        SystemManager systemManager;
        
        class TestSystem : public ISystem {
        public:
            bool initialize() override { return true; }
            void update(float) override {}
            void shutdown() override {}
            std::string getName() const override { return "TestSystem"; }
        };

        BENCHMARK("System Registration") {
            auto* system = systemManager.registerSystem<TestSystem>();
            return system != nullptr;
        };

        BENCHMARK("System Priority Updates") {
            auto* system = systemManager.registerSystem<TestSystem>(100);
            return system->getPriority();
        };
    }

    SECTION("Debug Manager Performance") {
        auto& debugManager = DebugManager::getInstance();
        
        BENCHMARK("Log Message Processing") {
            debugManager.log(LogLevel::Info, "Performance test message");
            return true;
        };

        BENCHMARK("Debug Overlay Control") {
            debugManager.setDebugOverlayEnabled(true);
            return debugManager.isDebugOverlayEnabled();
        };
    }

    app.shutdown();
}

TEST_CASE("Core Systems Stress Tests", "[core][stress]") {
    auto& app = Application::getInstance();
    app.initialize("StressTest");

    SECTION("System Manager Under Load") {
        SystemManager systemManager;
        std::vector<ISystem*> systems;

        struct StressTestSystem : public ISystem {
            int m_id{0};
            std::string m_name{"StressTestSystem"};

            bool initialize() override {
                m_name = "StressTestSystem" + std::to_string(m_id++);
                return true;
            }
            void update(float) override { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
            void shutdown() override {}
            std::string getName() const override { return m_name; }
        };

        // Register 100 systems
        REQUIRE_NOTHROW([&]() {
            for (int i = 0; i < 100; ++i) {
                auto* system = systemManager.registerSystem<StressTestSystem>();
                systems.push_back(system);
            }
        }());

        // Update all systems multiple times
        REQUIRE_NOTHROW([&]() {
            for (int i = 0; i < 10; ++i) {
                systemManager.update(1.0f / 60.0f);
            }
        }());
    }

    SECTION("Debug Manager Under Load") {
        auto& debugManager = DebugManager::getInstance();
        
        // Generate lots of log messages
        REQUIRE_NOTHROW([&]() {
            for (int i = 0; i < 1000; ++i) {
                debugManager.log(LogLevel::Info, "Stress test message " + std::to_string(i));
            }
        }());

        // Toggle debug overlay rapidly
        REQUIRE_NOTHROW([&]() {
            for (int i = 0; i < 1000; ++i) {
                debugManager.setDebugOverlayEnabled(i % 2 == 0);
            }
        }());
    }

    SECTION("Time Manager Precision Test") {
        TimeManager timeManager;
        std::vector<double> deltaTimes;
        
        // Collect delta times for statistical analysis
        for (int i = 0; i < 1000; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            timeManager.update();
            deltaTimes.push_back(timeManager.getDeltaTime());
        }

        // Calculate average and variance
        double sum = 0.0;
        for (double dt : deltaTimes) {
            sum += dt;
        }
        double avg = sum / deltaTimes.size();

        double variance = 0.0;
        for (double dt : deltaTimes) {
            variance += (dt - avg) * (dt - avg);
        }
        variance /= deltaTimes.size();

        // Check that timing is reasonably consistent
        REQUIRE(variance < 0.0001); // Adjust threshold based on system capabilities
    }

    app.shutdown();
} 