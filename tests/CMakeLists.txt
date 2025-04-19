# Include the GoogleTest module provided by CMake for CTest integration
include(GoogleTest)

function(add_transity_test TEST_NAME TEST_SOURCE TEST_LABEL)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main gmock LoggingLib ConfigLib)
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/googletest/googletest/include
        ${CMAKE_SOURCE_DIR}/lib/googletest/googlemock/include
        ${CMAKE_SOURCE_DIR}/include
    )
    gtest_add_tests(TARGET ${TEST_NAME} TEST_LIST current_tests)
    foreach(test_in_target ${current_tests})
        set_tests_properties(${test_in_target} PROPERTIES LABELS "${TEST_LABEL}")
    endforeach()
endfunction()

add_transity_test(LoggingSystemInitialization_test logging/logging_system_initialize.cpp "Logging")
add_transity_test(LoggingSystemLog_test logging/logging_system_log.cpp "Logging")
add_transity_test(LoggingSystemShutdown_test logging/logging_system_shutdown.cpp "Logging")

add_transity_test(ConfigSystem_test config/config_system_test.cpp "Config")