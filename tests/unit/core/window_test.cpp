#include <catch2/catch_test_macros.hpp>
#include "transity/core/window.hpp"

using namespace transity::core;

TEST_CASE("Window creation and configuration", "[window]") {
    SECTION("Default window configuration") {
        Window window;
        const auto& config = window.getConfig();
        
        REQUIRE(config.width == 1280);
        REQUIRE(config.height == 720);
        REQUIRE(config.title == "Transity");
        REQUIRE(config.fullscreen == false);
        REQUIRE(config.framerate == 60);
        REQUIRE(window.isOpen());
    }

    SECTION("Custom window configuration") {
        WindowConfig config;
        config.width = 800;
        config.height = 600;
        config.title = "Test Window";
        config.framerate = 30;
        
        Window window(config);
        const auto& actualConfig = window.getConfig();
        
        REQUIRE(actualConfig.width == 800);
        REQUIRE(actualConfig.height == 600);
        REQUIRE(actualConfig.title == "Test Window");
        REQUIRE(actualConfig.framerate == 30);
        REQUIRE(window.isOpen());
    }

    SECTION("Window reconfiguration") {
        Window window;
        WindowConfig newConfig;
        newConfig.width = 1024;
        newConfig.height = 768;
        newConfig.title = "Reconfigured Window";
        newConfig.framerate = 75;
        
        window.setConfig(newConfig);
        const auto& actualConfig = window.getConfig();
        
        REQUIRE(actualConfig.width == 1024);
        REQUIRE(actualConfig.height == 768);
        REQUIRE(actualConfig.title == "Reconfigured Window");
        REQUIRE(actualConfig.framerate == 75);
        REQUIRE(window.isOpen());
    }
}

TEST_CASE("Window rendering operations", "[window]") {
    Window window;
    
    SECTION("Frame operations") {
        REQUIRE_NOTHROW(window.beginFrame());
        REQUIRE_NOTHROW(window.endFrame());
    }
    
    SECTION("Window handle access") {
        REQUIRE_NOTHROW(window.getWindow());
    }
}

TEST_CASE("Window event processing", "[window]") {
    Window window;
    
    SECTION("Initial event processing") {
        REQUIRE(window.processEvents());
    }
}

TEST_CASE("Window resize events", "[window]") {
    Window window;
    bool resizeEventReceived = false;
    int newWidth = 0, newHeight = 0;
    
    window.setResizeCallback([&](int width, int height) {
        resizeEventReceived = true;
        newWidth = width;
        newHeight = height;
    });
    
    SECTION("Window resize event processing") {
        sf::Event resizeEvent;
        resizeEvent.type = sf::Event::Resized;
        resizeEvent.size.width = 1024;
        resizeEvent.size.height = 768;
        
        window.processEvent(resizeEvent);
        
        REQUIRE(resizeEventReceived);
        REQUIRE(newWidth == 1024);
        REQUIRE(newHeight == 768);
        
        const auto& config = window.getConfig();
        REQUIRE(config.width == 1024);
        REQUIRE(config.height == 768);
    }
}

TEST_CASE("Window focus events", "[window]") {
    Window window;
    bool focusEventReceived = false;
    bool hasFocus = false;
    
    window.setFocusCallback([&](bool focused) {
        focusEventReceived = true;
        hasFocus = focused;
    });
    
    SECTION("Window focus gained") {
        sf::Event focusEvent;
        focusEvent.type = sf::Event::GainedFocus;
        
        window.processEvent(focusEvent);
        
        REQUIRE(focusEventReceived);
        REQUIRE(hasFocus);
    }
    
    SECTION("Window focus lost") {
        sf::Event focusEvent;
        focusEvent.type = sf::Event::LostFocus;
        
        window.processEvent(focusEvent);
        
        REQUIRE(focusEventReceived);
        REQUIRE_FALSE(hasFocus);
    }
} 