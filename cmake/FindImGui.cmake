# FindImGui.cmake

find_path(IMGUI_INCLUDE_DIR
    NAMES imgui.h
    PATHS
        ${IMGUI_ROOT}/include
        $ENV{IMGUI_ROOT}/include
        ${CMAKE_SOURCE_DIR}/external/imgui
    DOC "ImGui include directory"
)

find_library(IMGUI_LIBRARY
    NAMES imgui
    PATHS
        ${IMGUI_ROOT}/lib
        $ENV{IMGUI_ROOT}/lib
        ${CMAKE_SOURCE_DIR}/external/imgui
    DOC "ImGui library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ImGui
    REQUIRED_VARS
        IMGUI_INCLUDE_DIR
)

if(ImGui_FOUND AND NOT TARGET ImGui::ImGui)
    add_library(ImGui::ImGui UNKNOWN IMPORTED)
    set_target_properties(ImGui::ImGui PROPERTIES
        IMPORTED_LOCATION "${IMGUI_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${IMGUI_INCLUDE_DIR}"
    )
endif() 