add_library(transity_core
    core/application.cpp
)

target_link_libraries(transity_core
    PUBLIC
        SFML::System
        SFML::Window
        SFML::Graphics
        imgui::imgui
        ImGui-SFML::ImGui-SFML
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        EnTT::EnTT
)

add_executable(transity main.cpp)
target_link_libraries(transity PRIVATE transity_core) 