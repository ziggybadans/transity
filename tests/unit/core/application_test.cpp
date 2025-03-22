#include <catch2/catch_test_macros.hpp>
#include "transity/core/application.hpp"

TEST_CASE("Application can be created and run", "[core]") {
    transity::core::Application app;
    REQUIRE_NOTHROW(app.run());
} 