add_library(transity_core
    core/application.cpp
    core/window.cpp
    core/window_manager.cpp
    core/input_manager.cpp
    core/time_manager.cpp
    core/system_manager.cpp
    core/debug_manager.cpp
    core/configuration_manager.cpp
    core/event_manager.cpp
)

add_library(transity_ecs
    ecs/entity_manager.cpp
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

target_link_libraries(transity_ecs
    PUBLIC
        transity_core
)

add_executable(transity main.cpp)
target_link_libraries(transity 
    PRIVATE 
        transity_core
        transity_ecs
) 