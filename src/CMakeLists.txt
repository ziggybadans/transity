add_library(transity_core
    core/application.cpp
    core/window.cpp
    core/input_manager.cpp
    core/time_manager.cpp
    core/system_manager.cpp
    core/debug_manager.cpp
)

target_link_libraries(transity_core
    PUBLIC
        sfml-system
        sfml-window
        sfml-graphics
        imgui::imgui
        ImGui-SFML::ImGui-SFML
        nlohmann_json::nlohmann_json
        EnTT::EnTT
)

add_executable(transity main.cpp)
target_link_libraries(transity PRIVATE transity_core) 