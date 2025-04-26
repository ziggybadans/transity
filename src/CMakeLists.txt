# Define a library target
# Add all source files belonging to this library here
file(GLOB LOGGING_SOURCES "logging/*.cpp")
add_library(LoggingLib ${LOGGING_SOURCES})
file(GLOB CONFIG_SOURCES "config/*.cpp")
add_library(ConfigLib ${CONFIG_SOURCES})
file(GLOB ECS_SOURCES "ecs/*.cpp")
add_library(ECSLib ${ECS_SOURCES})

# Specify include directories needed by this library
# PUBLIC: Means consumers linking to library also get these includes
# PRIVATE: Means only library uses these includes internally
# INTERFACE: Means only consumers linking to library get these includes
target_include_directories(LoggingLib PUBLIC
    # Allow code in this library to find headers in the main include dir
    ${CMAKE_SOURCE_DIR}/include
)
target_include_directories(ConfigLib PUBLIC
    # Allow code in this library to find headers in the main include dir
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/tomlplusplus/include
)
target_include_directories(ECSLib PUBLIC
    # Allow code in this library to find headers in the main include dir
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/entt/src
)

# Define the main application executable
# You can name it 'Transity', 'App', or whatever you prefer
add_executable(Transity main.cpp)

# Link the executable against the library we just defined
# This makes all the code in TransityLib available to the executable
target_link_libraries(Transity PRIVATE LoggingLib ConfigLib ECSLib)

# Link TransityLib against any dependencies it needs
# For example, if ConsoleLogger used SFML directly (it doesn't yet),
# you might add: target_link_libraries(TransityLib PRIVATE sfml-graphics)
# We'll add dependencies like EnTT, SFML, ImGui here as needed by the src code.