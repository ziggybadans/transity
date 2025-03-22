#include <catch2/catch_test_macros.hpp>
#include "transity/core/application.hpp"

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
        REQUIRE_THROWS_AS(app.initialize(), ApplicationError);
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
        REQUIRE_THROWS_AS(app.run(), ApplicationError);
    }
    
    SECTION("Run after initialization") {
        app.initialize();
        REQUIRE_NOTHROW(app.run());
        app.shutdown();
    }
} 