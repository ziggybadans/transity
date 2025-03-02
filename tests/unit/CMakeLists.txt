# Unit tests configuration

# Create test executable
add_executable(unit_tests
    test_main.cpp
    # Add more test files here as they are created
)

# Link with Google Test and project code
target_link_libraries(unit_tests PRIVATE
    gtest
    gtest_main
    # Add project libraries as needed
)

# Include directories
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(unit_tests) 