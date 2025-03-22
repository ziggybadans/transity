#include <catch2/catch_test_macros.hpp>
#include "transity/core/input_manager.hpp"

using namespace transity::core;

TEST_CASE("InputManager Singleton", "[core][input]") {
    SECTION("getInstance returns same instance") {
        auto& instance1 = InputManager::getInstance();
        auto& instance2 = InputManager::getInstance();
        REQUIRE(&instance1 == &instance2);
    }
}

TEST_CASE("InputManager Keyboard Input", "[core][input]") {
    auto& input = InputManager::getInstance();
    
    // Reset state before each section
    input.clear();
    input.update();
    
    SECTION("Key press and release") {
        sf::Event keyPress;
        keyPress.type = sf::Event::KeyPressed;
        keyPress.key.code = sf::Keyboard::A;
        
        sf::Event keyRelease;
        keyRelease.type = sf::Event::KeyReleased;
        keyRelease.key.code = sf::Keyboard::A;

        input.processEvent(keyPress);
        REQUIRE(input.isKeyPressed(sf::Keyboard::A));
        REQUIRE(input.isKeyJustPressed(sf::Keyboard::A));
        REQUIRE_FALSE(input.isKeyJustReleased(sf::Keyboard::A));

        input.update(); // Move to next frame
        REQUIRE(input.isKeyPressed(sf::Keyboard::A));
        REQUIRE_FALSE(input.isKeyJustPressed(sf::Keyboard::A));
        REQUIRE_FALSE(input.isKeyJustReleased(sf::Keyboard::A));

        input.processEvent(keyRelease);
        REQUIRE_FALSE(input.isKeyPressed(sf::Keyboard::A));
        REQUIRE_FALSE(input.isKeyJustPressed(sf::Keyboard::A));
        REQUIRE(input.isKeyJustReleased(sf::Keyboard::A));

        input.update(); // Move to next frame
        REQUIRE_FALSE(input.isKeyPressed(sf::Keyboard::A));
        REQUIRE_FALSE(input.isKeyJustPressed(sf::Keyboard::A));
        REQUIRE_FALSE(input.isKeyJustReleased(sf::Keyboard::A));
    }
}

TEST_CASE("InputManager Mouse Input", "[core][input]") {
    auto& input = InputManager::getInstance();
    
    // Reset state before each section
    input.clear();
    input.update();
    
    SECTION("Mouse button press and release") {
        sf::Event mousePress;
        mousePress.type = sf::Event::MouseButtonPressed;
        mousePress.mouseButton.button = sf::Mouse::Left;
        
        sf::Event mouseRelease;
        mouseRelease.type = sf::Event::MouseButtonReleased;
        mouseRelease.mouseButton.button = sf::Mouse::Left;

        input.processEvent(mousePress);
        REQUIRE(input.isMouseButtonPressed(sf::Mouse::Left));
        REQUIRE(input.isMouseButtonJustPressed(sf::Mouse::Left));
        REQUIRE_FALSE(input.isMouseButtonJustReleased(sf::Mouse::Left));

        input.update(); // Move to next frame
        REQUIRE(input.isMouseButtonPressed(sf::Mouse::Left));
        REQUIRE_FALSE(input.isMouseButtonJustPressed(sf::Mouse::Left));
        REQUIRE_FALSE(input.isMouseButtonJustReleased(sf::Mouse::Left));

        input.processEvent(mouseRelease);
        REQUIRE_FALSE(input.isMouseButtonPressed(sf::Mouse::Left));
        REQUIRE_FALSE(input.isMouseButtonJustPressed(sf::Mouse::Left));
        REQUIRE(input.isMouseButtonJustReleased(sf::Mouse::Left));

        input.update(); // Move to next frame
        REQUIRE_FALSE(input.isMouseButtonPressed(sf::Mouse::Left));
        REQUIRE_FALSE(input.isMouseButtonJustPressed(sf::Mouse::Left));
        REQUIRE_FALSE(input.isMouseButtonJustReleased(sf::Mouse::Left));
    }

    SECTION("Mouse movement") {
        // Reset state before mouse movement test
        input.clear();
        input.update();

        sf::Event mouseMove;
        mouseMove.type = sf::Event::MouseMoved;
        mouseMove.mouseMove.x = 100;
        mouseMove.mouseMove.y = 200;

        input.processEvent(mouseMove);
        auto pos = input.getMousePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);

        // Delta should be 0 since this is the first movement
        auto delta = input.getMouseDelta();
        REQUIRE(delta.x == 0);
        REQUIRE(delta.y == 0);

        // Move mouse again in the same frame
        mouseMove.mouseMove.x = 150;
        mouseMove.mouseMove.y = 250;
        input.processEvent(mouseMove);

        // Delta should reflect the movement within this frame
        delta = input.getMouseDelta();
        REQUIRE(delta.x == 50);
        REQUIRE(delta.y == 50);

        // Move to next frame
        input.update();

        // Move mouse in new frame
        mouseMove.mouseMove.x = 200;
        mouseMove.mouseMove.y = 300;
        input.processEvent(mouseMove);

        // Delta should reflect movement from last position
        delta = input.getMouseDelta();
        REQUIRE(delta.x == 50);
        REQUIRE(delta.y == 50);
    }

    SECTION("Mouse wheel") {
        // Reset state before mouse wheel test
        input.clear();
        input.update();

        sf::Event wheelScroll;
        wheelScroll.type = sf::Event::MouseWheelScrolled;
        wheelScroll.mouseWheelScroll.delta = 1.5f;

        input.processEvent(wheelScroll);
        REQUIRE(input.getMouseWheelDelta() == 1.5f);

        input.update();
        input.clear(); // Should reset wheel delta
        REQUIRE(input.getMouseWheelDelta() == 0.0f);
    }
} 