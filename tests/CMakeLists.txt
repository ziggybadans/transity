# Include the GoogleTest module provided by CMake for CTest integration
include(GoogleTest)

function(add_transity_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main gmock TransityLib)
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/googletest/googletest/include
        ${CMAKE_SOURCE_DIR}/lib/googletest/googlemock/include
        ${CMAKE_SOURCE_DIR}/include
    )
    gtest_add_tests(TARGET ${TEST_NAME})
endfunction()

add_transity_test(ConsoleLogger_test logging/ConsoleLogger_test.cpp)
add_transity_test(FileLogger_test logging/FileLogger_test.cpp)