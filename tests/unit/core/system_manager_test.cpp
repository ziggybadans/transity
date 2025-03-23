#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <thread>
#include <chrono>
#include "transity/core/system_manager.hpp"

using namespace transity::core;

// Resource class for testing ResourceHandle
class TestResource {
public:
    TestResource(int value) : m_value(value) {}
    int getValue() const { return m_value; }
    void setValue(int value) { m_value = value; }
private:
    int m_value;
};

// Mock system for testing
class MockSystem : public ISystem {
public:
    MockSystem(const std::string& name = "MockSystem") : m_name(name) {}

    bool initialize() override { 
        m_initialized = true;
        
        // Simulate work for async testing
        if (m_simulateWorkDuration > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_simulateWorkDuration));
        }
        
        return m_initSuccess; 
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
    void setSimulateWorkDuration(int milliseconds) { m_simulateWorkDuration = milliseconds; }
    void setInitSuccess(bool success) { m_initSuccess = success; }

private:
    std::string m_name;
    bool m_initialized = false;
    int m_updateCount = 0;
    float m_lastDeltaTime = 0.0f;
    bool m_shutdownCalled = false;
    std::function<void()> m_onUpdate;
    int m_simulateWorkDuration = 0;
    bool m_initSuccess = true;
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

TEST_CASE("SystemManager asynchronous loading", "[system]") {
    SystemManager manager;

    SECTION("Asynchronous initialization") {
        auto* system1 = manager.registerSystem<MockSystem>();
        system1->setName("System1");
        system1->setSimulateWorkDuration(500); // 500ms work

        auto* system2 = manager.registerSystem<MockSystem>();
        system2->setName("System2");
        system2->setSimulateWorkDuration(500); // 500ms work

        // Start initialization asynchronously
        auto future = manager.initializeAsync();
        
        // Give enough time for the async process to start
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Systems should be marked as loading now
        REQUIRE(manager.isLoadingAsync());
        
        // Wait for completion and check result
        REQUIRE(future.get());
        
        // Systems should no longer be loading
        REQUIRE_FALSE(manager.isLoadingAsync());
        
        // Both systems should be initialized
        REQUIRE(system1->wasInitialized());
        REQUIRE(system2->wasInitialized());
    }

    SECTION("Asynchronous system tasks") {
        auto* system = manager.registerSystem<MockSystem>();
        
        // Schedule an async task
        auto future = system->scheduleAsyncTask([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return 42;
        });
        
        // Wait for result
        int result = future.get();
        REQUIRE(result == 42);
    }

    SECTION("Failed initialization") {
        auto* system = manager.registerSystem<MockSystem>();
        system->setInitSuccess(false);
        
        auto future = manager.initializeAsync();
        REQUIRE_FALSE(future.get());
    }
}

TEST_CASE("ResourceHandle functionality", "[system]") {
    SystemManager manager;
    
    SECTION("Create and use resource handle") {
        // Create a resource
        auto resource = std::make_unique<TestResource>(42);
        
        // Create a handle to the resource
        auto handle = manager.createResourceHandle<TestResource>(1, resource.get());
        
        // Verify handle properties
        REQUIRE(handle.isValid());
        REQUIRE(handle.getId() == 1);
        REQUIRE(handle.get() == resource.get());
        REQUIRE(handle->getValue() == 42);
        
        // Test operators
        REQUIRE((*handle).getValue() == 42);
        REQUIRE((bool)handle);
        
        // Modify through handle
        handle->setValue(100);
        REQUIRE(resource->getValue() == 100);
    }
    
    SECTION("Invalid handle") {
        auto handle = ResourceHandle<TestResource>();
        REQUIRE_FALSE(handle.isValid());
        REQUIRE(handle.getId() == 0);
        REQUIRE(handle.get() == nullptr);
        REQUIRE_FALSE((bool)handle);
    }
} 