# Include the GoogleTest module provided by CMake for CTest integration
include(GoogleTest)

function(add_transity_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main gmock LoggingLib)
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/googletest/googletest/include
        ${CMAKE_SOURCE_DIR}/lib/googletest/googlemock/include
        ${CMAKE_SOURCE_DIR}/include
    )
    gtest_add_tests(TARGET ${TEST_NAME})
endfunction()

add_transity_test(LoggingSystemInitialization_test logging/logging_system_initialize.cpp)
add_transity_test(LoggingSystemLog_test logging/logging_system_log.cpp)