#include <iostream>
#include "transity/core/application.hpp"

int main() {
    try {
        auto& app = transity::core::Application::getInstance();
        app.initialize("Transity - Transport Management Simulator");
        app.run();
        app.shutdown();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
} 