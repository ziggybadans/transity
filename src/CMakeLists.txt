# Define a library target named TransityLib
# Add all source files belonging to this library here
add_library(TransityLib
    logging/ConsoleLogger.cpp
    logging/LogUtils.cpp
)

# Specify include directories needed by this library
# PUBLIC: Means consumers linking to TransityLib also get these includes
# PRIVATE: Means only TransityLib uses these includes internally
# INTERFACE: Means only consumers linking to TransityLib get these includes
target_include_directories(TransityLib PUBLIC
    # Allow code in this library to find headers in the main include dir
    ${CMAKE_SOURCE_DIR}/include
)

# Link TransityLib against any dependencies it needs
# For example, if ConsoleLogger used SFML directly (it doesn't yet),
# you might add: target_link_libraries(TransityLib PRIVATE sfml-graphics)
# We'll add dependencies like EnTT, SFML, ImGui here as needed by the src code.