# Find the GTest package (this might already be implicitly available
# due to add_subdirectory in the parent scope, but being explicit is safer)
# find_package(GTest REQUIRED) # Often not needed if added via add_subdirectory

# Include the GoogleTest module provided by CMake for CTest integration
include(GoogleTest)

# Add your test executable
# This creates an executable named 'basic_test' from 'basic_test.cpp'
add_executable(basic_test basic_test.cpp)
add_executable(ConsoleLogger_test logging/ConsoleLogger_test.cpp)

# Link the test executable against Google Test libraries
# The exact target names might be gtest/gtest_main or GTest::gtest/GTest::gtest_main
# depending on the GTest version and how it was built/added.
# Using the namespaced versions (GTest::*) is generally preferred if available.
# Let's assume the non-namespaced ones based on the add_subdirectory approach.
target_link_libraries(basic_test PRIVATE gtest gtest_main)
target_link_libraries(ConsoleLogger_test PRIVATE gtest gtest_main gmock Transity)
target_include_directories(ConsoleLogger_test PRIVATE ${CMAKE_SOURCE_DIR}/lib/googletest/googletest/include)
target_include_directories(ConsoleLogger_test PRIVATE ${CMAKE_SOURCE_DIR}/lib/googletest/googlemock/include) 
target_include_directories(ConsoleLogger_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Discover and add tests to CTest
# This command automatically finds tests defined with TEST() or TEST_F()
# in the 'basic_test' executable and adds them to CTest.
gtest_add_tests(TARGET basic_test)
gtest_add_tests(TARGET ConsoleLogger_test)