add_library(transity_core
    core/application.cpp
    core/window.cpp
    core/input_manager.cpp
)

target_link_libraries(transity_core
    PUBLIC
        sfml-system
        sfml-window
        sfml-graphics
        imgui::imgui
        ImGui-SFML::ImGui-SFML
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        EnTT::EnTT
)

add_executable(transity main.cpp)
target_link_libraries(transity PRIVATE transity_core) 