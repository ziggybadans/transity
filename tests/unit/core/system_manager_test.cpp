#include <catch2/catch_test_macros.hpp>
#include <functional>
#include "transity/core/system_manager.hpp"

using namespace transity::core;

// Mock system for testing
class MockSystem : public ISystem {
public:
    MockSystem(const std::string& name = "MockSystem") : m_name(name) {}

    bool initialize() override { 
        m_initialized = true;
        return true; 
    }
    
    void update(float deltaTime) override {
        m_updateCount++;
        m_lastDeltaTime = deltaTime;
        if (m_onUpdate) m_onUpdate();
    }
    
    void shutdown() override {
        m_shutdownCalled = true;
    }
    
    std::string getName() const override { return m_name; }

    // Test helper methods
    bool wasInitialized() const { return m_initialized; }
    int getUpdateCount() const { return m_updateCount; }
    float getLastDeltaTime() const { return m_lastDeltaTime; }
    bool wasShutdown() const { return m_shutdownCalled; }
    void setName(const std::string& name) { m_name = name; }
    void setUpdateCallback(std::function<void()> callback) { m_onUpdate = callback; }

private:
    std::string m_name;
    bool m_initialized = false;
    int m_updateCount = 0;
    float m_lastDeltaTime = 0.0f;
    bool m_shutdownCalled = false;
    std::function<void()> m_onUpdate;
};

TEST_CASE("SystemManager basic functionality", "[system]") {
    SystemManager manager;

    SECTION("System registration") {
        auto* system = manager.registerSystem<MockSystem>();
        REQUIRE(system != nullptr);
        REQUIRE(system->getName() == "MockSystem");
    }

    SECTION("System retrieval by type") {
        auto* registered = manager.registerSystem<MockSystem>();
        auto* retrieved = manager.getSystem<MockSystem>();
        REQUIRE(retrieved == registered);
    }

    SECTION("System retrieval by name") {
        auto* registered = manager.registerSystem<MockSystem>();
        auto* retrieved = manager.getSystem("MockSystem");
        REQUIRE(retrieved == registered);
    }

    SECTION("System initialization") {
        auto* system = manager.registerSystem<MockSystem>();
        REQUIRE(manager.initialize());
        REQUIRE(system->wasInitialized());
    }

    SECTION("System update") {
        auto* system = manager.registerSystem<MockSystem>();
        manager.initialize();
        manager.update(0.016f);
        REQUIRE(system->getUpdateCount() == 1);
        REQUIRE(system->getLastDeltaTime() == 0.016f);
    }

    SECTION("System enable/disable") {
        auto* system = manager.registerSystem<MockSystem>();
        manager.initialize();
        
        system->setEnabled(false);
        manager.update(0.016f);
        REQUIRE(system->getUpdateCount() == 0);

        system->setEnabled(true);
        manager.update(0.016f);
        REQUIRE(system->getUpdateCount() == 1);
    }

    SECTION("System shutdown") {
        auto* system = manager.registerSystem<MockSystem>();
        manager.initialize();
        manager.shutdown();
        REQUIRE(system->wasShutdown());
    }
}

TEST_CASE("SystemManager priority ordering", "[system]") {
    SystemManager manager;
    std::vector<std::string> updateOrder;

    SECTION("Systems update in priority order") {
        // Register systems with different priorities
        auto* highPriority = manager.registerSystem<MockSystem>();
        highPriority->setPriority(2);
        highPriority->setName("HighPriority");
        highPriority->setUpdateCallback([&updateOrder]() {
            updateOrder.push_back("HighPriority");
        });

        auto* mediumPriority = manager.registerSystem<MockSystem>();
        mediumPriority->setPriority(1);
        mediumPriority->setName("MediumPriority");
        mediumPriority->setUpdateCallback([&updateOrder]() {
            updateOrder.push_back("MediumPriority");
        });

        auto* lowPriority = manager.registerSystem<MockSystem>();
        lowPriority->setPriority(0);
        lowPriority->setName("LowPriority");
        lowPriority->setUpdateCallback([&updateOrder]() {
            updateOrder.push_back("LowPriority");
        });

        manager.initialize();
        manager.update(0.016f);

        REQUIRE(updateOrder.size() == 3);
        REQUIRE(updateOrder[0] == "LowPriority");
        REQUIRE(updateOrder[1] == "MediumPriority");
        REQUIRE(updateOrder[2] == "HighPriority");
    }
} 