add_library(transity_core
    core/application.cpp
    ecs/entity_manager.cpp
    gui/gui_manager.cpp
    rendering/renderer.cpp
    simulation/simulator.cpp
    util/logger.cpp
)

target_link_libraries(transity_core
    PUBLIC
        sfml-system
        sfml-window
        sfml-graphics
        ImGui::ImGui
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        EnTT::EnTT
)

add_executable(transity main.cpp)
target_link_libraries(transity PRIVATE transity_core) 